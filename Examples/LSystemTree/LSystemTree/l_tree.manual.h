#include "GiglDefaultInclude.h"

#include "TreeSegment.h"

class LTree_Node;
class TreePart;

class LTree_Node
{
public:
	virtual ~LTree_Node() {}
	Vector2D start_pos;
	Vector2D end_pos;
	virtual void Draw() = 0;
};

class TreePart : public LTree_Node
{
public:
	virtual ~TreePart() {}
};

class ntTree : public TreePart
{
public:
	ntTree(TreePart* _bottom, TreePart* _right, TreePart* _left, TreePart* _top)
		: bottom(_bottom), right(_right), left(_left), top(_top)
	{
		start_pos = bottom->start_pos;
		end_pos = top->end_pos;
	}
	virtual ~ntTree()
	{
		delete bottom;
		if (right) delete right;
		if (left) delete left;
		delete top;
	}
	TreePart* bottom;
	TreePart* right;
	TreePart* left;
	TreePart* top;
	void Draw()
	{
		bottom->Draw();
		top->Draw();
		if (right) right->Draw();
		if (left) left->Draw();
	}
};

class termTree : public TreePart
{
public:
	termTree(TreeSegment* _seg) : seg(_seg)
	{
		start_pos = seg->start;
		end_pos = seg->end;
	}
	virtual ~termTree() { delete seg; }
	TreeSegment* seg;
	void Draw() { seg->Draw(); }
};

enum ConfigInstance { DETERM_TREE, ND_TREE, ADV_ND_TREE };

double ntFac(int d, int n, double w, double max_w)
{
	return d < n ? 0.5 + w/max_w : 0.0;
}

double brchFac(double w, double max_w)
{
	return  2 - 3*w/max_w;
}

double getBranchDeg(ConfigInstance instance)
{
	switch (instance)
	{
	case DETERM_TREE:
		return 45.0;
	case ND_TREE:
		return GetRandFloat(30.0, 60.0);
	case ADV_ND_TREE:
		return GetRandFloat(30.0, 60.0);
	}
}

TreePart* Generate_TreePart(int depth, const Vector2D& start, const Vector2D& dir, double bottom_w, double top_w, int n, double unit_len, double max_wid, ConfigInstance instance)
{
	double probs[2];
	double branch_prob;
	switch (instance)
	{
	case DETERM_TREE:
		probs[0] = depth < n;
		probs[1] = 1.0 - probs[0];
		branch_prob = 1.0;
		break;
	case ND_TREE:
		probs[0] = depth < n ? 0.8 : 0.0;
		probs[1] = 1.0 - probs[0];
		branch_prob = 0.7;
		break;
	case ADV_ND_TREE:
		probs[0] = ntFac(depth, n, bottom_w, max_wid);
		probs[1] = 1.0 - probs[0];
		branch_prob = brchFac(top_w, max_wid);
		break;
	}
	NonegativeProbs(probs, 2);
	NormalizeProbs(probs, 2);
	int choice = GetRandChoiceFromProbs(probs, 2);

	switch (choice)
	{
	case 0:
	{
		double mid_w = 0.5 * (bottom_w + top_w);
		TreePart* bottom = Generate_TreePart(depth + 1, start, dir, bottom_w, mid_w, n, unit_len, max_wid, instance);
		TreePart* right = (RandomRoll(branch_prob) ? Generate_TreePart(depth + 1, bottom->end_pos, RotateDegrees(dir, -getBranchDeg(instance)), mid_w, 0.0, n, unit_len, max_wid, instance) : nullptr);
		TreePart* left = (RandomRoll(branch_prob) ? Generate_TreePart(depth + 1, bottom->end_pos, RotateDegrees(dir, getBranchDeg(instance)), mid_w, 0.0, n, unit_len, max_wid, instance) : nullptr);
		TreePart* top = Generate_TreePart(depth + 1, bottom->end_pos, dir, mid_w, top_w, n, unit_len, max_wid, instance);
		
		return new ntTree(bottom, right, left, top);
	}
	case 1:
	{
		TreeSegment* seg = TermSegGen(depth, start, dir, bottom_w, top_w, n, unit_len);

		return new termTree(seg);
	}
	}

	return nullptr;
}

class LTree
{
public:
	LTree(int _n, double _unit_len, double _max_wid, ConfigInstance _instance)
		: root(Generate_TreePart(0, MkVector2D(0.0, 0.0), MkVector2D(0.0, 1.0), _max_wid, 0.0, _n, _unit_len, _max_wid, _instance)) {}
	~LTree() { delete root; }
	LTree_Node* root;
	void Draw() { root->Draw(); }
};

LTree* GenerateDetermLTree(int n, double unit_len)
{
	double max_wid = (double)(1 << n);
	return new LTree(n, unit_len, max_wid, DETERM_TREE);
}

LTree* GenerateNDLTree(int n, double unit_len)
{
	double max_wid = (double)(1 << n);
	return new LTree(n, unit_len, max_wid, ND_TREE);
}

LTree* GenerateAdvNDLTree(int n, double unit_len)
{
	double max_wid = (double)(1 << n);
	return new LTree(n, unit_len, max_wid, ADV_ND_TREE);
}
