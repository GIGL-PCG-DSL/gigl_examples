#include "GiglDefaultInclude.h"

#include "DungeonSpace.h"

union RoomPtr
{
	Room* r;
	BossRoom* br;
};

struct RoomEntry
{
	RoomEntry() : type(0) { ptr.r = nullptr; }
	RoomEntry(Room* _r) : type(0) { ptr.r = _r; }
	RoomEntry(BossRoom* _br) : type(1) { ptr.br = _br; }
	void operator= (RoomEntry&& in)
	{
		type = in.type;
		ptr = in.ptr;
		in.type = 0;
		in.ptr.r = nullptr;
	}
	~RoomEntry()
	{
		if (type == 0)
		{
			if (ptr.r) delete ptr.r;
		}
		else
		{
			if (ptr.br) delete ptr.br;
		}
	}
	int type;
	RoomPtr ptr;
	Range2D GetRange() const
	{
		if (type == 0)
			return ptr.r->range;
		else
			return ptr.br->range;
	}
	void Draw(bool is_padded)
	{
		if (type == 0)
			ptr.r->Draw(is_padded);
		else
			ptr.br->Draw(is_padded);
	}
};

struct CorridEntry
{
	CorridEntry() : ptr(nullptr) {}
	CorridEntry(int _index, bool _is_horz, double _pos, const Range1D& _cover) : index(_index), is_horz(_is_horz), pos(_pos), cover(_cover), ptr(nullptr) {}
	~CorridEntry() { if (ptr) delete ptr; }
	int index;
	bool is_horz;
	double pos;
	Range1D cover;
	Corridor* ptr;
	void UpdateHorzContact(double y, const Range2D& range, Range1D& x_cover) const
	{
		if (range.top >= y && range.bottom <= y)
		{
			if (range.right <= pos)
			{
				if (range.right > x_cover.min)
					x_cover.min = range.right;
			}
			else if (range.left >= pos)
			{
				if (range.left < x_cover.max)
					x_cover.max = range.left;
			}
		}
	}
	Range1D GetHorzContact(double y, const RoomEntry* room_list, const CorridEntry* corrid_list, int room_num, const Range2D& whole_range) const
	{
		Range1D x_cover(whole_range.ExtractXRange());
		for (int i = 0; i < room_num; i++)
		{
			Range2D range = room_list[i].GetRange();
			UpdateHorzContact(y, range, x_cover);
		}
		for (int i = 0; i < index; i++)
		{
			Range2D range = corrid_list[i].GetRange();
			UpdateHorzContact(y, range, x_cover);
		}
		return x_cover;
	}
	void UpdateVertContact(double x, const Range2D& range, Range1D& y_cover) const
	{
		if (range.right >= x && range.left <= x)
		{
			if (range.top <= pos)
			{
				if (range.top > y_cover.min)
					y_cover.min = range.top;
			}
			else if (range.bottom >= pos)
			{
				if (range.bottom < y_cover.max)
					y_cover.max = range.bottom;
			}
		}
	}
	Range1D GetVertContact(double x, const RoomEntry* room_list, const CorridEntry* corrid_list, int room_num, const Range2D& whole_range) const
	{
		Range1D y_cover(whole_range.ExtractYRange());
		for (int i = 0; i < room_num; i++)
		{
			Range2D range = room_list[i].GetRange();
			UpdateVertContact(x, range, y_cover);
		}
		for (int i = 0; i < index; i++)
		{
			Range2D range = corrid_list[i].GetRange();
			UpdateVertContact(x, range, y_cover);
		}
		return y_cover;
	}
	void Finalize(const RoomEntry* room_list, const CorridEntry* corrid_list, int room_num, const Range2D& whole_range)
	{
		if (is_horz)
		{
			Range1D x_cover_top(GetHorzContact(cover.min, room_list, corrid_list, room_num, whole_range));
			Range1D x_cover_bottom(GetHorzContact(cover.max, room_list, corrid_list, room_num, whole_range));
			Range1D x_cover = UnionRange1D(x_cover_top, x_cover_bottom);
			ptr = new Corridor(x_cover.min, cover.min, x_cover.max, cover.max);
		}
		else
		{
			Range1D y_cover_top(GetVertContact(cover.min, room_list, corrid_list, room_num, whole_range));
			Range1D y_cover_bottom(GetVertContact(cover.max, room_list, corrid_list, room_num, whole_range));
			Range1D y_cover = UnionRange1D(y_cover_top, y_cover_bottom);
			ptr = new Corridor(cover.min, y_cover.min, cover.max, y_cover.max);
		}
	}
	Range2D GetRange() const
	{
		return ptr->range;
	}
	void Draw(bool is_padded)
	{
		ptr->Draw(is_padded);
	}
};

class BspDungeon
{
public:
	BspDungeon(const Range2D& _whole_range, double _unit_min, double _unit_max, double _wall_min, double _have_boss_area) : room_num(0), corrid_num(0)
	{
		int max_num = (int)(_whole_range.span_x/_unit_min) * (int)(_whole_range.span_y/_unit_min);
		room_list = new RoomEntry[max_num];
		corrid_list = new CorridEntry[max_num - 1];
		Generate_DungeonArea(_whole_range, MkNoConstraintRange1D(), MkNoConstraintRange1D(), _have_boss_area, _unit_min, _unit_max, _wall_min);
		for (int i = 0; i < corrid_num; i++)
			corrid_list[i].Finalize(room_list, corrid_list, room_num, _whole_range);
	}
	~BspDungeon()
	{
		delete[] room_list;
		delete[] corrid_list;
	}
	RoomEntry* room_list;
	CorridEntry* corrid_list;
	int room_num;
	int corrid_num;
	Range2D Generate_DungeonArea(const Range2D& range, const Range1D& cover_x, const Range1D& cover_y, bool have_boss, double unit_min, double unit_max, double wall_min)
	{
		double probs[4] = { 0.3, 0.3, 0.4, 0.0 };
		if (range.span_x < 2 * unit_min)
			probs[1] = 0.0;
		if (range.span_y < 2 * unit_min)
			probs[0] = 0.0;
		if (range.span_x > unit_max || range.span_y > unit_max)
			probs[2] = 0.0;
		else if (have_boss)
		{
			probs[3] = 0.4;
			probs[2] = 0.0;
		}

		NormalizeProbs(probs, 4);
		int choice = GetRandChoiceFromProbs(probs, 4);

		switch (choice)
		{
		case 0:
		{
			double corrid_min = 0.1 * unit_min;
			double corrid_max = 0.2 * unit_min;

			Range2D b_range;
			Range2D t_range;

			double corrid_size = GetRandFloat(corrid_min, corrid_max);
			double corrid_pos = GetRandFloat(range.left + wall_min, range.right - wall_min - corrid_size);
			Range1D corrid_cover = MkRange1DFromMin(corrid_pos, corrid_size);
			Range1D union_cover = UnionRange1D(corrid_cover, cover_x);
			double subarea_max = range.span_y - unit_min;
			double bottom_size = GetRandFloat(unit_min, (unit_max < subarea_max ? unit_max : subarea_max));
			double separator_y = range.bottom + bottom_size;
			bool boss_in_bottom = false, boss_in_top = false;
			if (RandomRoll(0.5))
				boss_in_bottom = have_boss;
			else
				boss_in_top = have_boss;
			if (RandomRoll(0.5))
			{
				b_range = Generate_DungeonArea(MkRange2D(range.left, range.bottom, range.right, separator_y), union_cover, MkMinConstraintRange1D(cover_y.min), boss_in_bottom, unit_min, unit_max, wall_min);
				t_range = Generate_DungeonArea(MkRange2D(range.left, separator_y, range.right, range.top), corrid_cover, MkMaxConstraintRange1D(cover_y.max), boss_in_top, unit_min, unit_max, wall_min);
			}
			else
			{
				b_range = Generate_DungeonArea(MkRange2D(range.left, range.bottom, range.right, separator_y), corrid_cover, MkMinConstraintRange1D(cover_y.min), boss_in_bottom, unit_min, unit_max, wall_min);
				t_range = Generate_DungeonArea(MkRange2D(range.left, separator_y, range.right, range.top), union_cover, MkMaxConstraintRange1D(cover_y.max), boss_in_top, unit_min, unit_max, wall_min);
			}

			corrid_list[corrid_num] = CorridEntry(corrid_num, false, separator_y, corrid_cover);
			corrid_num++;

			return UnionRange2D(b_range, t_range);
		}
		case 1:
		{
			double corrid_min = 0.1 * unit_min;
			double corrid_max = 0.2 * unit_min;

			Range2D l_range;
			Range2D r_range;

			double corrid_size = GetRandFloat(corrid_min, corrid_max);
			double corrid_pos = GetRandFloat(range.bottom + wall_min, range.top - wall_min - corrid_size);
			Range1D corrid_cover = MkRange1DFromMin(corrid_pos, corrid_size);
			Range1D union_cover = UnionRange1D(corrid_cover, cover_y);
			double subarea_max = range.span_x - unit_min;
			double left_size = GetRandFloat(unit_min, (unit_max < subarea_max ? unit_max : subarea_max));
			double separator_x = range.left + left_size;
			bool boss_in_left = false, boss_in_right = false;
			if (RandomRoll(0.5))
				boss_in_left = have_boss;
			else
				boss_in_right = have_boss;
			if (RandomRoll(0.5))
			{
				l_range = Generate_DungeonArea(MkRange2D(range.left, range.bottom, separator_x, range.top), MkMinConstraintRange1D(cover_x.min), union_cover, boss_in_left, unit_min, unit_max, wall_min);
				r_range = Generate_DungeonArea(MkRange2D(separator_x, range.bottom, range.right, range.top), MkMaxConstraintRange1D(cover_x.max), corrid_cover, boss_in_right, unit_min, unit_max, wall_min);
			}
			else
			{
				l_range = Generate_DungeonArea(MkRange2D(range.left, range.bottom, separator_x, range.top), MkMinConstraintRange1D(cover_x.min), corrid_cover, boss_in_left, unit_min, unit_max, wall_min);
				r_range = Generate_DungeonArea(MkRange2D(separator_x, range.bottom, range.right, range.top), MkMaxConstraintRange1D(cover_x.max), union_cover, boss_in_right, unit_min, unit_max, wall_min);
			}

			corrid_list[corrid_num] = CorridEntry(corrid_num, true, separator_x, corrid_cover);
			corrid_num++;

			return UnionRange2D(l_range, r_range);
		}
		case 2:
		{
			double room_min = 0.7 * range.GetMinSpan();

			Room* room = TermRoomGen(range, cover_x, cover_y, room_min, wall_min);
			room_list[room_num++] = RoomEntry(room);

			return room->range;
		}
		case 3:
		{
			double room_min = 0.7 * range.GetMinSpan();

			BossRoom* room = BossRoomGen(range, cover_x, cover_y, room_min, wall_min);
			room_list[room_num++] = RoomEntry(room);

			return room->range;
		}
		}
	}
	void Draw(bool is_padded)
	{
		for (int i = 0; i < room_num; i++)
		{
			room_list[i].Draw(is_padded);
		}
		for (int i = 0; i < corrid_num; i++)
		{
			corrid_list[i].Draw(is_padded);
		}
	}
	void Draw()
	{
		Draw(true);
		Draw(false);
	}
};

BspDungeon* GenerateDungeon(const Range2D& whole_range, double unit_min, double unit_max, double wall_min, double have_boss_area)
{
	return new BspDungeon(whole_range, unit_min, unit_max, wall_min, have_boss_area);
}
