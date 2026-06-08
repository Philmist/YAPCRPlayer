#pragma once



namespace utl {

typedef enum {
	DIRECTION_NONE,
	DIRECTION_LEFT,
	DIRECTION_RIGHT,
	DIRECTION_UP,
	DIRECTION_DOWN,
} DIRECTION;

std::wstring getGestureText(const std::vector<int>& gesture);

class Gesture {
	int movement_;
	std::vector<int> gesture_;
	int dir_;
	CPoint pt_; 

public:
	Gesture();
	virtual ~Gesture();

	void start(int movement, const CPoint& pt);
	void move(const CPoint& pt);
	void get(std::vector<int>& gesture);
	bool empty() const;
};

} // gesture