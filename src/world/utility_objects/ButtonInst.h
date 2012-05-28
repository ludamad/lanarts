#ifndef BUTTONINST_H_
#define BUTTONINST_H_

#include "../objects/GameInst.h"
#include <string>

typedef void (*callbackf)(void* data);

class ButtonInst: public GameInst {
public:
	enum {RADIUS = 10};
	ButtonInst(const std::string& str, const BBox& bounding, callbackf callback = NULL, void* data = NULL) :
		GameInst(bounding.x1, bounding.y1, RADIUS), str(str), bounding(bounding), callback(callback), data(data){}
	virtual ~ButtonInst(){}
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ButtonInst* clone() const;

private:
	std::string str;
	BBox bounding;
	callbackf callback;
	void* data;
};

#endif /* BUTTONINST_H_ */
