#include "GiglDefaultInclude.h"

#include "DungeonSpace.h"

class DungeonArea
{
public:
	virtual ~DungeonArea() {}
	Range2D room_range;
	virtual double GetContactFromLeft(double y) = 0;
	virtual double GetContactFromBottom(double x) = 0;
	virtual double GetContactFromRight(double y) = 0;
	virtual double GetContactFromTop(double x) = 0;
	virtual void Draw(bool is_padded) = 0;
};

class hDivide : public DungeonArea
{
public:
	hDivide(DungeonArea* _b_area, DungeonArea* _t_area, Corridor* _corridor, const Range2D& _range) : b_area(_b_area), t_area(_t_area), corridor(_corridor)
	{
		room_range = UnionRange2D(UnionRange2D(b_area->room_range, t_area->room_range), corridor->range);
	}
	~hDivide()
	{
		delete b_area;
		delete t_area;
		delete corridor;
	}
	DungeonArea* b_area;
	DungeonArea* t_area;
	Corridor* corridor;
	double GetContactFromLeft(double y)
	{
		double contact = 1e10;
		if (y <= b_area->room_range.top)
			contact = b_area->GetContactFromLeft(y);
		else if (y >= t_area->room_range.bottom)
			contact = t_area->GetContactFromLeft(y);
		return (contact <= corridor->range.left || y <= corridor->range.bottom || y >= corridor->range.top ? contact : corridor->range.left);
	}
	double GetContactFromBottom(double x)
	{
		if (x >= b_area->room_range.left && x < b_area->room_range.right)
			return b_area->GetContactFromBottom(x);
		return t_area->GetContactFromBottom(x);
	}
	double GetContactFromRight(double y)
	{
		double contact = -1e10;
		if (y <= b_area->room_range.top)
			contact = b_area->GetContactFromRight(y);
		else if (y >= t_area->room_range.bottom)
			contact = t_area->GetContactFromRight(y);
		return (contact >= corridor->range.right || y <= corridor->range.bottom || y >= corridor->range.top ? contact : corridor->range.right);
	}
	double GetContactFromTop(double x)
	{
		if (x >= t_area->room_range.left && x < t_area->room_range.right)
			return t_area->GetContactFromTop(x);
		return b_area->GetContactFromTop(x);
	}
	void Draw(bool is_padded)
	{
		b_area->Draw(is_padded);
		t_area->Draw(is_padded);
		corridor->Draw(is_padded);
	}
};

class vDivide : public DungeonArea
{
public:
	vDivide(DungeonArea* _l_area, DungeonArea* _r_area, Corridor* _corridor, const Range2D& _range) : l_area(_l_area), r_area(_r_area), corridor(_corridor)
	{
		room_range = UnionRange2D(UnionRange2D(l_area->room_range, r_area->room_range), corridor->range);
	}
	~vDivide()
	{
		delete l_area;
		delete r_area;
		delete corridor;
	}
	DungeonArea* l_area;
	DungeonArea* r_area;
	Corridor* corridor;
	double wall_min;
	double GetContactFromLeft(double y)
	{
		if (y >= l_area->room_range.bottom && y < l_area->room_range.top)
			return l_area->GetContactFromLeft(y);
		return r_area->GetContactFromLeft(y);
	}
	double GetContactFromBottom(double x)
	{
		double contact = 1e10;
		if (x <= l_area->room_range.right)
			contact = l_area->GetContactFromBottom(x);
		else if (x >= r_area->room_range.left)
			contact = r_area->GetContactFromBottom(x);
		return (contact <= corridor->range.bottom || x <= corridor->range.left || x >= corridor->range.right ? contact : corridor->range.bottom);
	}
	double GetContactFromRight(double y)
	{
		if (y >= r_area->room_range.bottom && y < r_area->room_range.top)
			return r_area->GetContactFromRight(y);
		return l_area->GetContactFromRight(y);
	}
	double GetContactFromTop(double x)
	{
		double contact = -1e10;
		if (x <= l_area->room_range.right)
			contact = l_area->GetContactFromTop(x);
		else if (x >= r_area->room_range.left)
			contact = r_area->GetContactFromTop(x);
		return (contact >= corridor->range.top || x <= corridor->range.left || x >= corridor->range.right ? contact : corridor->range.top);
	}
	void Draw(bool is_padded)
	{
		l_area->Draw(is_padded);
		r_area->Draw(is_padded);
		corridor->Draw(is_padded);
	}
};

class tArea : public DungeonArea
{
public:
	tArea(Room* _room, const Range2D& _range) : room(_room)
	{
		room_range = room->range;
	}
	~tArea() { delete room; }
	Room* room;
	double GetContactFromLeft(double y) { return room_range.left; }
	double GetContactFromBottom(double x) { return room_range.bottom; }
	double GetContactFromRight(double y) { return room_range.right; }
	double GetContactFromTop(double x) { return room_range.top; }
	void Draw(bool is_padded) { room->Draw(is_padded); }
};

class bossArea : public DungeonArea
{
public:
	bossArea(BossRoom* _room, const Range2D& _range) : room(_room)
	{
		room_range = room->range;
	}
	~bossArea() { delete room; }
	BossRoom* room;
	double GetContactFromLeft(double y) { return room_range.left; }
	double GetContactFromBottom(double x) { return room_range.bottom; }
	double GetContactFromRight(double y) { return room_range.right; }
	double GetContactFromTop(double x) { return room_range.top; }
	void Draw(bool is_padded) { room->Draw(is_padded); }
};

DungeonArea* Generate_DungeonArea(const Range2D& range, const Range1D& cover_x, const Range1D& cover_y, bool have_boss, double unit_min, double unit_max, double wall_min)
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

		DungeonArea* b_area;
		DungeonArea* t_area;
		Corridor* corridor;

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
			b_area = Generate_DungeonArea(MkRange2D(range.left, range.bottom, range.right, separator_y), union_cover, MkMinConstraintRange1D(cover_y.min), boss_in_bottom, unit_min, unit_max, wall_min);
			t_area = Generate_DungeonArea(MkRange2D(range.left, separator_y, range.right, range.top), corrid_cover, MkMaxConstraintRange1D(cover_y.max), boss_in_top, unit_min, unit_max, wall_min);
		}
		else
		{
			b_area = Generate_DungeonArea(MkRange2D(range.left, range.bottom, range.right, separator_y), corrid_cover, MkMinConstraintRange1D(cover_y.min), boss_in_bottom, unit_min, unit_max, wall_min);
			t_area = Generate_DungeonArea(MkRange2D(range.left, separator_y, range.right, range.top), union_cover, MkMaxConstraintRange1D(cover_y.max), boss_in_top, unit_min, unit_max, wall_min);
		}

		double corrid_bottom_left = b_area->GetContactFromTop(corrid_cover.min);
		double corrid_bottom_right = b_area->GetContactFromTop(corrid_cover.max);
		double corrid_bottom = (corrid_bottom_left < corrid_bottom_right ? corrid_bottom_left : corrid_bottom_right);
		double corrid_top_left = t_area->GetContactFromBottom(corrid_cover.min);
		double corrid_top_right = t_area->GetContactFromBottom(corrid_cover.max);
		double corrid_top = (corrid_top_left > corrid_top_right ? corrid_top_left : corrid_top_right);
		corridor = new Corridor(corrid_cover.min, corrid_bottom, corrid_cover.max, corrid_top);

		return new hDivide(b_area, t_area, corridor, range);
	}
	case 1:
	{
		double corrid_min = 0.1 * unit_min;
		double corrid_max = 0.2 * unit_min;

		DungeonArea* l_area;
		DungeonArea* r_area;
		Corridor* corridor;

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
			l_area = Generate_DungeonArea(MkRange2D(range.left, range.bottom, separator_x, range.top), MkMinConstraintRange1D(cover_x.min), union_cover, boss_in_left, unit_min, unit_max, wall_min);
			r_area = Generate_DungeonArea(MkRange2D(separator_x, range.bottom, range.right, range.top), MkMaxConstraintRange1D(cover_x.max), corrid_cover, boss_in_right, unit_min, unit_max, wall_min);
		}
		else
		{
			l_area = Generate_DungeonArea(MkRange2D(range.left, range.bottom, separator_x, range.top), MkMinConstraintRange1D(cover_x.min), corrid_cover, boss_in_left, unit_min, unit_max, wall_min);
			r_area = Generate_DungeonArea(MkRange2D(separator_x, range.bottom, range.right, range.top), MkMaxConstraintRange1D(cover_x.max), union_cover, boss_in_right, unit_min, unit_max, wall_min);
		}

		double corrid_left_bottom = l_area->GetContactFromRight(corrid_cover.min);
		double corrid_left_top = l_area->GetContactFromRight(corrid_cover.max);
		double corrid_left = (corrid_left_bottom < corrid_left_top ? corrid_left_bottom : corrid_left_top);
		double corrid_right_bottom = r_area->GetContactFromLeft(corrid_cover.min);
		double corrid_right_top = r_area->GetContactFromLeft(corrid_cover.max);
		double corrid_right = (corrid_right_bottom > corrid_right_top ? corrid_right_bottom : corrid_right_top);
		corridor = new Corridor(corrid_left, corrid_cover.min, corrid_right, corrid_cover.max);

		return new vDivide(l_area, r_area, corridor, range);
	}
	case 2:
	{
		double room_min = 0.7 * range.GetMinSpan();

		Room* room = TermRoomGen(range, cover_x, cover_y, room_min, wall_min);

		return new tArea(room, range);
	}
	case 3:
	{
		double room_min = 0.7 * range.GetMinSpan();

		BossRoom* room = BossRoomGen(range, cover_x, cover_y, room_min, wall_min);

		return new bossArea(room, range);
	}
	}

	return nullptr;
}

class BspDungeon
{
public:
	BspDungeon(const Range2D& _whole_range, double _unit_min, double _unit_max, double _wall_min, double _have_boss_area)
		: root(Generate_DungeonArea(_whole_range, MkNoConstraintRange1D(), MkNoConstraintRange1D(), _have_boss_area, _unit_min, _unit_max, _wall_min)) {}
	~BspDungeon() { delete root; }
	DungeonArea* root;
	void Draw()
	{
		root->Draw(true);
		root->Draw(false);
	}
};

BspDungeon* GenerateDungeon(const Range2D& whole_range, double unit_min, double unit_max, double wall_min, double have_boss_area)
{
	return new BspDungeon(whole_range, unit_min, unit_max, wall_min, have_boss_area);
}
