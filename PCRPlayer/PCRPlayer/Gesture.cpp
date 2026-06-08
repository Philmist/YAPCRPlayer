#include "stdafx.h"

namespace utl {

std::wstring getGestureText(const std::vector<int>& gesture)
{
	std::wstring tmp;
	for (auto it = gesture.begin(); it != gesture.end(); ++it)
	{
		switch (*it)
		{
		default:
		case DIRECTION_NONE:	tmp += L"ÅH"; break;
		case DIRECTION_LEFT:	tmp += L"Å©"; break;
		case DIRECTION_RIGHT:	tmp += L"Å®"; break;
		case DIRECTION_UP:		tmp += L"Å™"; break;
		case DIRECTION_DOWN:	tmp += L"Å´"; break;
		}
	}
	return tmp;
}



Gesture::Gesture()
	: movement_(24)
	, dir_(DIRECTION_NONE)
{}

Gesture::~Gesture()
{}


void Gesture::start(int movement, const CPoint& pt)
{
	movement_ = movement;
	gesture_.clear();
	dir_ = DIRECTION_NONE;
	pt_ = pt;
	//DebugText(L"Gesture: start\n");
}

void Gesture::move(const CPoint& pt)
{
	int mx = pt_.x - pt.x;
	int my = pt_.y - pt.y;

	//DebugText(L"mx:%d my:%d\n", abs(mx), abs(my));

	if ((abs(mx) > movement_) && (abs(mx) >= abs(my)))
	{
		if (mx > 0)
		{// ç∂
			if (dir_ == DIRECTION_LEFT) { pt_ = pt; return; }
			dir_ = DIRECTION_LEFT;
			//DebugText(L"Gesture: Å©\n");
		}
		else
		{// âE
			if (dir_ == DIRECTION_RIGHT) { pt_ = pt; return; }
			dir_ = DIRECTION_RIGHT;
			//DebugText(L"Gesture: Å®\n");
		}
	}
	else if ((abs(my) > movement_) && (abs(mx) < abs(my)))
	{
		if (my > 0)
		{// è„
			if (dir_ == DIRECTION_UP) { pt_ = pt; return; }
			dir_ = DIRECTION_UP;
			//DebugText(L"Gesture: Å™\n");
		}
		else
		{// â∫
			if (dir_ == DIRECTION_DOWN) { pt_ = pt; return; }
			dir_ = DIRECTION_DOWN;
			//DebugText(L"Gesture: Å´\n");
		}
	}
	else
	{
		return;
	}

	pt_ = pt;
	gesture_.push_back(dir_);
}


void Gesture::get(std::vector<int>& gesture)
{
	gesture = gesture_;
}

bool Gesture::empty() const
{
	return gesture_.empty();
}

} // gesture
