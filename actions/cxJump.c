//
//  cxJump.c
//  cxEngine
//
//  Created by xuhua on 10/25/13.
//  Copyright (c) 2013 xuhua. All rights reserved.
//

#include "cxJump.h"

static void cxJumpInit(cxAny pav)
{
    cxJump this = pav;
    CX_ASSERT(this->super.view != NULL, "view not set");
    this->prevPos = this->startPos = this->super.view->position;
}

static void cxJumpStep(cxAny pav,cxFloat dt,cxFloat time)
{
    cxJump this = pav;
    cxFloat frac = fmodf( time * this->jumps, 1.0f );
    cxFloat y = this->height * 4 * frac * (1 - frac) + this->position.y * time;
    cxFloat x = this->position.x * time;
    cxVec2f diff;
    cxVec2f currPos = this->super.view->position;
    kmVec2Subtract(&diff, &currPos, &this->prevPos);
    kmVec2Add(&this->startPos, &diff, &this->startPos);
    cxVec2f nPos;
    kmVec2Add(&nPos, &this->startPos, &cxVec2fv(x, y));
    this->prevPos = nPos;
    cxViewSetPosition(this->super.view, nPos);
}

static cxBool cxJumpXMLReadAttr(cxAny xmlAction,cxAny mAction, xmlTextReaderPtr reader)
{
    cxActionXMLReadAttr(xmlAction, mAction, reader);
    cxJump this = mAction;
    cxXMLReadFloatsAttr(reader, "cxJump.position", &this->position.x);
    this->height = cxXMLReadFloatAttr(reader, "cxJump.height", this->height);
    this->jumps  = cxXMLReadIntAttr(reader, "cxJump.jumps", this->jumps);
    return true;
}

CX_OBJECT_INIT(cxJump, cxAction)
{
    cxObjectSetXMLReadFunc(this, cxJumpXMLReadAttr);
    CX_METHOD_SET(this->super.Init, cxJumpInit);
    CX_METHOD_SET(this->super.Step, cxJumpStep);
}
CX_OBJECT_FREE(cxJump, cxAction)
{

}
CX_OBJECT_TERM(cxJump, cxAction)

cxJump cxJumpCreate(cxFloat duration,cxVec2f position,cxFloat height,cxUInt jumps)
{
    cxJump this = CX_CREATE(cxJump);
    this->super.duration = duration;
    this->position = position;
    this->height = height;
    this->jumps = jumps;
    return this;
}

