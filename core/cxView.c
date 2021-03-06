//
//  cxView.c
//  cxEngine
//
//  Created by xuhua on 10/2/13.
//  Copyright (c) 2013 xuhua. All rights reserved.
//

#include <kazmath/matrix.h>
#include "cxView.h"
#include "cxEngine.h"
#include "cxOpenGL.h"
#include "cxAction.h"
#include "cxViewXML.h"

//al at ar ab
static void cxViewXMLReadAutoResize(cxAny view,xmlTextReaderPtr reader)
{
    cxView this = view;
    cxChar *sal = cxXMLAttr("cxView.left");
    cxChar *sar = cxXMLAttr("cxView.right");
    cxChar *sat = cxXMLAttr("cxView.top");
    cxChar *sab = cxXMLAttr("cxView.bottom");
    if(sal != NULL){
        this->autoMask |= cxViewAutoResizeLeft;
        this->autoBox.l = atof(sal) * this->fixscale.x;
    }
    if(sar != NULL){
        this->autoMask |= cxViewAutoResizeRight;
        this->autoBox.r = atof(sar) * this->fixscale.x;
    }
    if(sat != NULL){
        this->autoMask |= cxViewAutoResizeTop;
        this->autoBox.t = atof(sat) * this->fixscale.y;
    }
    if(sab != NULL){
        this->autoMask |= cxViewAutoResizeBottom;
        this->autoBox.b = atof(sab) * this->fixscale.y;
    }
    if(cxXMLReadBoolAttr(reader, "cxView.fill", false)){
        this->autoMask = cxViewAutoResizeFill;
        this->autoBox = cxBox4fv(0, 0, 0, 0);
    }
    xmlFree(sal);
    xmlFree(sar);
    xmlFree(sat);
    xmlFree(sab);
}

static void cxViewXMLReadRectToView(cxAny view,xmlTextReaderPtr reader)
{
    cxView this = view;
    cxChar *sx = cxXMLAttr("cxView.x");
    cxChar *sy = cxXMLAttr("cxView.y");
    cxChar *sz = cxXMLAttr("cxView.z");
    cxChar *sw = cxXMLAttr("cxView.w");
    cxChar *sh = cxXMLAttr("cxView.h");
    cxVec2f pos = this->position;
    if(sx != NULL){
        pos.x = atof(sx);
    }
    if(sy != NULL){
        pos.y = atof(sy);
    }
    cxViewSetPosition(view, pos);
    if(sz != NULL){
        cxViewSetOrder(view, atoi(sz));
    }
    cxSize2f size = this->size;
    if(sw != NULL){
        size.w = atof(sw);
    }
    if(sh != NULL){
        size.h = atof(sh);
    }
    cxViewSetSize(view, size);
    xmlFree(sx);
    xmlFree(sy);
    xmlFree(sz);
    xmlFree(sw);
    xmlFree(sh);
}

cxBool cxViewZeroSize(cxAny pview)
{
    cxView this = pview;
    return cxSize2Zero(this->size);
}

static void cxViewFixScaleAttr(cxAny view,xmlTextReaderPtr reader)
{
    cxView this = view;
    cxEngine engine = cxEngineInstance();
    //need set engine->dessize
    cxChar *fixscale = cxXMLAttr("cxView.fixScale");
    if(cxConstCharsEqu(fixscale, "w")){
        cxViewSetFixScale(view, cxVec2fv(engine->scale.x, engine->scale.x));
    }else if(cxConstCharsEqu(fixscale, "h")){
        cxViewSetFixScale(view, cxVec2fv(engine->scale.y, engine->scale.y));
    }else if(cxConstCharsEqu(fixscale, "wh")){
        cxViewSetFixScale(view, engine->scale);
    }else{
        cxViewSetFixScale(view, cxXMLReadVec2fAttr(reader, "cxView.fixScale", this->fixscale));
    }
    xmlFree(fixscale);
}

cxBool cxViewXMLReadAttr(cxAny pxml,cxAny view, xmlTextReaderPtr reader)
{
    cxObjectXMLReadAttr(pxml, view, reader);
    cxViewXML xml = pxml;
    cxView this = view;
    //fixscale
    cxViewFixScaleAttr(view, reader);
    //rect
    cxViewXMLReadRectToView(view,reader);
    //resize
    cxViewXMLReadAutoResize(view, reader);
    //cropping
    cxViewSetCropping(view,cxXMLReadBoolAttr(reader, "cxView.cropping", this->isCropping));
    //top
    cxViewSetTop(view, cxXMLReadBoolAttr(reader, "cxView.hideTop", this->hideTop));
    //anchor
    cxViewSetAnchor(view, cxXMLReadVec2fAttr(reader, "cxView.anchor", this->anchor));
    //scale
    cxViewSetScale(view, cxXMLReadVec2fAttr(reader, "cxView.scale", this->scale));
    //color
    cxColor4f color = this->color;
    cxXMLReadFloatsAttr(reader, "cxView.color", &color.r);
    cxViewSetColor(view, cxColor3fv(color.r, color.g, color.b));
    cxViewSetAlpha(view, cxXMLReadFloatAttr(reader, "cxView.alpha", color.a));
    //visible
    cxViewSetVisible(view, cxXMLReadBoolAttr(reader, "cxView.visible", this->isVisible));
    //debug border
    cxViewSetBorder(view, cxXMLReadBoolAttr(reader, "cxView.border", this->isBorder));
    //rotate raxis
    cxVec3f raxis;
    if(cxXMLReadFloatsAttr(reader, "cxView.raxis", &raxis.x) == 3){
        cxViewSetRaxis(view, raxis);
    }
    //rotate degress
    cxFloat degrees;
    if(cxXMLReadFloatsAttr(reader, "cxView.degrees", &degrees) == 1){
        cxViewSetDegrees(view, degrees);
    }
    //view event
    cxXMLAppendEvent(xml->events, this, cxView, onEnter);
    cxXMLAppendEvent(xml->events, this, cxView, onExit);
    cxXMLAppendEvent(xml->events, this, cxView, onUpdate);
    cxXMLAppendEvent(xml->events, this, cxView, onResize);
    cxXMLAppendEvent(xml->events, this, cxView, onLayout);
    cxXMLAppendEvent(xml->events, this, cxView, onChanged);
    cxXMLAppendEvent(xml->events, this, cxView, onDirty);
    return true;
}

void cxViewSetViewEvent(cxEvent *event)
{
    CX_RETURN(event->args == NULL || event->sender == NULL);
    cxConstChars viewid = cxEventArgString(event->args, "view");
    cxView pview = cxViewXMLGet(event->sender, viewid);
    CX_RETURN(pview == NULL);
    cxConstChars scale = cxEventArgString(event->args, "scale");
    if(scale != NULL){
        cxVec2f vscale = pview->scale;
        cxReadFloats(scale, &vscale.x);
        cxViewSetScale(pview, vscale);
    }
    cxConstChars size = cxEventArgString(event->args, "size");
    if(size != NULL){
        cxSize2f vsize = pview->size;
        cxReadFloats(size, &vsize.w);
        cxViewSetSize(pview, vsize);
    }
    cxConstChars position = cxEventArgString(event->args, "position");
    if(position != NULL){
        cxVec2f vposition = pview->position;
        cxReadFloats(position, &vposition.x);
        cxViewSetPosition(pview, vposition);
    }
    cxConstChars degress = cxEventArgString(event->args, "degress");
    if(degress != NULL){
        cxViewSetDegrees(pview, atof(degress));
    }
    cxConstChars anchor = cxEventArgString(event->args, "anchor");
    if(anchor != NULL){
        cxVec2f vanchor = pview->anchor;
        cxReadFloats(scale, &vanchor.x);
        cxViewSetAnchor(pview, vanchor);
    }
}

void cxViewSetCropping(cxAny pview,cxBool cropping)
{
    cxView this = pview;
    this->isCropping = cropping;
}

CX_OBJECT_INIT(cxView, cxObject)
{
    this->super.cxBase = cxBaseTypeView;
    this->hideTop = true;
    this->isBorder = false;
    this->isVisible = true;
    this->isDirty = true;
    this->color = cxColor4fv(1.0f, 1.0f, 1.0f, 1.0f);
    this->size = cxSize2fv(0.0f, 0.0f);
    this->anchor = cxVec2fv(0.5f, 0.5f);
    this->raxis = cxVec3fv(0.0f, 0.0f, 1.0f);
    this->scale = cxVec2fv(1.0f, 1.0f);
    this->fixscale = cxVec2fv(1.0f, 1.0f);
    this->subViews = CX_ALLOC(cxList);
    CX_METHOD_SET(this->IsTouch, cxViewIsTouch);
    cxObjectSetXMLReadFunc(this, cxViewXMLReadAttr);
    this->actions = CX_ALLOC(cxHash);
    this->caches = CX_ALLOC(cxHash);
}
CX_OBJECT_FREE(cxView, cxObject)
{
    CX_EVENT_RELEASE(this->onDirty);
    CX_EVENT_RELEASE(this->onChanged);
    CX_EVENT_RELEASE(this->onEnter);
    CX_EVENT_RELEASE(this->onExit);
    CX_EVENT_RELEASE(this->onUpdate);
    CX_EVENT_RELEASE(this->onResize);
    CX_EVENT_RELEASE(this->onLayout);
    CX_RELEASE(this->subViews);
    CX_RELEASE(this->actions);
    CX_RELEASE(this->caches);
    CX_RELEASE(this->args);
}
CX_OBJECT_TERM(cxView, cxObject)

void cxViewSetCache(cxAny pview,cxConstChars key,cxAny object)
{
    cxView this = pview;
    CX_ASSERT(object != NULL, "args error");
    cxHashSet(this->caches, cxHashStrKey(key), object);
}

cxAny cxViewGetCache(cxAny pview,cxConstChars key)
{
    cxView this = pview;
    return cxHashGet(this->caches, cxHashStrKey(key));
}

void cxViewSetArgs(cxAny pview,cxAny args)
{
    cxView this = pview;
    CX_RETAIN_SWAP(this->args, args);
}

cxVec2f cxViewPosition(cxAny pview)
{
    cxView this = pview;
    return this->position;
}

cxSize2f cxViewSize(cxAny pview)
{
    cxView this = pview;
    return this->size;
}

cxBox4f cxViewBox(cxAny pview)
{
    cxView this = pview;
    cxFloat wh = this->size.w/2.0f;
    cxFloat hh = this->size.h/2.0f;
    return cxBox4fv(-wh, wh, hh, -hh);
}

cxAny cxViewArgs(cxAny pview)
{
    cxView this = pview;
    return this->args;
}

cxColor4f cxViewColor(cxAny pview)
{
    cxView this = pview;
    return this->color;
}

cxBool cxViewContainsGLBox(cxAny pview)
{
    cxEngine engine = cxEngineInstance();
    cxRect4f vr = cxViewGLRect(pview);
    cxRect4f gr = cxRect4fv(0, 0, engine->winsize.w, engine->winsize.h);
    return cxRect4fContainsRect4f(gr,vr);
}

cxRect4f cxViewGLRect(cxAny pview)
{
    cxView this = pview;
    cxFloat wh = this->size.w/2.0f;
    cxFloat hh = this->size.h/2.0f;
    cxVec2f p1 = cxVec2fv(-wh, -hh);
    cxVec2f p2 = cxVec2fv(wh, hh);
    p1 = cxViewPointToGLPoint(pview, p1);
    p2 = cxViewPointToGLPoint(pview, p2);
    return cxRect4fv(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);
}

void cxViewSetTop(cxAny pview,cxBool top)
{
    cxView this = pview;
    this->hideTop = top;
}

void cxViewSetAutoResizeBox(cxAny pview,cxBox4f box)
{
    cxView this = pview;
    this->autoBox = box;
}

void cxViewSetAutoResizeMask(cxAny pview,cxViewAutoResizeMask mask)
{
    cxView this = pview;
    this->autoMask = mask;
}

void cxViewSetDirty(cxAny pview,cxBool dirty)
{
    cxView this = pview;
    this->isDirty = dirty;
}

void cxViewOnUpdate(cxAny pview,cxEventFunc func)
{
    cxView this = pview;
    CX_EVENT_APPEND(this->onUpdate, func, NULL);
}

void cxViewSetBorder(cxAny pview,cxBool border)
{
    cxView this = pview;
    this->isBorder = border;
}

cxVec2f cxWindowPointToGLPoint(cxVec2f wPoint)
{
    cxEngine engine = cxEngineInstance();
    cxFloat x = wPoint.x + engine->winsize.w/2.0f;
    cxFloat y = wPoint.y + engine->winsize.h/2.0f;
    return cxVec2fv(x, y);
}

cxVec2f cxGLPointToWindowPoint(cxVec2f glPoint)
{
    cxEngine engine = cxEngineInstance();
    cxFloat x = glPoint.x - engine->winsize.w/2.0f;
    cxFloat y = glPoint.y - engine->winsize.h/2.0f;
    return cxVec2fv(x, y);
}

cxVec2f cxViewPointToGLPoint(cxAny pview,cxVec2f pos)
{
    cxVec2f ret = cxViewPointToWindowPoint(pview, pos);
    return cxWindowPointToGLPoint(ret);
}

cxVec2f cxGLPointToViewPoint(cxAny pview,cxVec2f pos)
{
    cxVec2f ret = cxGLPointToWindowPoint(pos);
    return cxWindowPointToViewPoint(pview,ret);
}

cxVec2f cxViewPointToWindowPoint(cxAny pview,cxVec2f vPoint)
{
    cxView this = pview;
    cxView pv = this;
    cxVec3f out;
    kmVec3Fill(&out, vPoint.x, vPoint.y, 0);
    while (pv != NULL && pv->parentView != NULL) {
        kmVec3Transform(&out, &out, &pv->anchorMatrix);
        kmVec3Transform(&out, &out, &pv->normalMatrix);
        pv = pv->parentView;
    }
    return cxVec2fv(out.x, out.y);
}

cxVec2f cxWindowPointToViewPoint(cxAny pview,cxVec2f wPoint)
{
    cxView this = pview;
    cxView pv = this;
    cxVec3f out;
    cxMatrix4f matrix;
    kmVec3Fill(&out, wPoint.x, wPoint.y, 0);
    cxArray list = CX_ALLOC(cxArray);
    while (pv != NULL && pv->parentView != NULL) {
        cxArrayAppend(list, pv);
        pv = pv->parentView;
    }
    CX_ARRAY_REVERSE(list, ele){
        pv = cxArrayObject(ele);
        kmMat4Inverse(&matrix, &pv->normalMatrix);
        kmVec3Transform(&out, &out, &matrix);
        kmMat4Inverse(&matrix, &pv->anchorMatrix);
        kmVec3Transform(&out, &out, &matrix);
    }
    CX_RELEASE(list);
    return cxVec2fv(out.x, out.y);
}

void cxViewSetSize(cxAny pview,cxSize2f size)
{
    cxView this = pview;
    CX_RETURN(cxSize2fEqu(this->size, size));
    this->size = size;
    this->isDirty = true;
    CX_EVENT_FIRE(this, onResize);
}

static int cxViewSortByZOrder(cxListElement *lp,cxListElement *rp)
{
    cxView v1 = (cxView)lp->object;
    cxView v2 = (cxView)rp->object;
    return v1->zorder - v2->zorder;
}

void cxViewSort(cxAny pview)
{
    cxView this = pview;
    cxListSort(this->subViews, cxViewSortByZOrder);
    this->isSort = false;
}

void cxViewSetVisible(cxAny pview,cxBool visible)
{
    cxView this = pview;
    this->isVisible = visible;
}

cxAny cxViewGetParentView(cxAny pview)
{
    CX_ASSERT(pview != NULL, "args error");
    cxView this = pview;
    return this->parentView;
}

void cxViewSetColor(cxAny pview,cxColor3f color)
{
    cxView this = pview;
    if(!kmAlmostEqual(this->color.r, color.r)){
        this->color.r = color.r;
        this->isDirty = true;
    }
    if(!kmAlmostEqual(this->color.g, color.g)){
        this->color.g = color.g;
        this->isDirty = true;
    }
    if(!kmAlmostEqual(this->color.b, color.b)){
        this->color.b = color.b;
        this->isDirty = true;
    }
}

void cxViewSetAlpha(cxAny pview,cxFloat alpha)
{
    cxView this = pview;
    CX_RETURN(kmAlmostEqual(this->color.a, alpha));
    this->color.a = alpha;
    this->isDirty = true;
}

void cxViewSetOrder(cxAny pview,cxInt order)
{
    cxView this = pview;
    CX_RETURN(this->zorder == order);
    this->zorder = order;
    if(this->parentView != NULL){
        this->parentView->isSort = true;
    }
}

void cxViewSetPosition(cxAny pview,cxVec2f position)
{
    cxView this = pview;
    CX_RETURN(cxVec2fEqu(this->position, position));
    this->position = position;
    this->isDirty = true;
}

void cxViewSetAnchor(cxAny pview,cxVec2f anchor)
{
    cxView this = pview;
    CX_RETURN(cxVec2fEqu(this->anchor, anchor));
    this->anchor = anchor;
    this->isDirty = true;
}

void cxViewSetFixScale(cxAny pview,cxVec2f scale)
{
    cxView this = pview;
    CX_RETURN(cxVec2fEqu(this->fixscale, scale));
    this->fixscale = scale;
    this->isDirty = true;
}

void cxViewSetScale(cxAny pview,cxVec2f scale)
{
    cxView this = pview;
    CX_RETURN(cxVec2fEqu(this->scale, scale));
    this->scale = scale;
    this->isDirty = true;
}

void cxViewSetDegrees(cxAny pview,cxFloat degrees)
{
    cxViewSetRadians(pview,kmDegreesToRadians(degrees));
}

void cxViewSetRaxis(cxAny pview,cxVec3f raxis)
{
    cxView this = pview;
    CX_RETURN(kmVec3AreEqual(&this->raxis, &raxis));
    this->raxis = raxis;
    this->isDirty = true;
}

void cxViewSetRadians(cxAny pview,cxFloat radians)
{
    cxView this = pview;
    CX_RETURN(cxFloatEqu(this->radians,radians));
    this->radians = radians;
    this->isDirty = true;
}

cxVec2f cxViewScale(cxAny pview)
{
    cxView this = pview;
    return cxVec2fv(this->fixscale.x * this->scale.x, this->fixscale.y * this->scale.y);
}


void cxViewTransform(cxAny pview)
{
    cxView this = pview;
    CX_RETURN(!this->isDirty);
    cxMatrix4f transMatrix;
    kmMat4Translation(&transMatrix, this->position.x, this->position.y, 0.0f);
    
    cxMatrix4f rotateMatrix;
    kmMat4RotationAxisAngle(&rotateMatrix, &this->raxis, this->radians);
    
    cxVec2f scale = cxViewScale(this);
    cxMatrix4f scaleMatrix;
    kmMat4Scaling(&scaleMatrix, scale.x, scale.y, 1.0f);
    
    kmMat4Multiply(&this->normalMatrix, &transMatrix, &rotateMatrix);
    kmMat4Multiply(&this->normalMatrix, &this->normalMatrix, &scaleMatrix);
    
    cxFloat x = this->size.w * (0.5f - this->anchor.x);
    cxFloat y = this->size.h * (0.5f - this->anchor.y);
    kmMat4Translation(&this->anchorMatrix, x, y, 0);
    
    CX_EVENT_FIRE(this, onDirty);
    //
    if(this->isCropping){
        this->scissor = cxViewGLRect(pview);
    }
    this->isDirty = false;
}

static void cxViewDrawBorder(cxAny pview)
{
    cxView this = pview;
    CX_RETURN(!cxEngineInstance()->isShowBorder && !this->isBorder);
    cxBox4f b = cxViewBox(this);
    cxBoxVec2f box = cxBoxVec2fFromBox4f(b);
    cxDrawLineBox(&box, cxRED);
}

void cxViewEnter(cxAny pview)
{
    cxView this = pview;
    this->isRunning = true;
    CX_EVENT_FIRE(this, onEnter);
    CX_LIST_FOREACH(this->subViews, ele){
        cxView view = ele->object;
        if(view->isRunning){
            continue;
        }
        cxViewEnter(view);
    }
}

void cxViewExit(cxAny pview)
{
    cxView this = pview;
    CX_LIST_FOREACH(this->subViews, ele){
        cxView view = ele->object;
        if(!view->isRunning){
            continue;
        }
        cxViewExit(view);
    }
    CX_EVENT_FIRE(this, onExit);
    this->isRunning = false;
}

void cxViewAppend(cxAny pview,cxAny newview)
{
    CX_ASSERT(pview != NULL && newview != NULL, "parent view or new view null");
    cxView this = pview;
    cxView new = newview;
    CX_ASSERT(newview != NULL && new->subElement == NULL, "newview null or add to view");
    new->subElement = cxListAppend(this->subViews, new);
    new->parentView = this;
    if(this->isRunning){
        cxViewEnter(new);
        cxViewLayout(new);
    }
    CX_EVENT_FIRE(this, onChanged);
}

void cxViewAutoResizing(cxAny pview)
{
    cxView this = pview;
    CX_RETURN(this->parentView == NULL || this->autoMask == cxViewAutoResizeNone);
    cxView parent = this->parentView;
    cxSize2f size = this->size;
    cxVec2f pos = this->position;
    cxViewAutoResizeMask mask = this->autoMask;
    cxVec2f scale = cxViewScale(this);
    if((mask & cxViewAutoResizeLeft) && (mask & cxViewAutoResizeRight)){
        size.w = parent->size.w - this->autoBox.l - this->autoBox.r;
        this->scale.x = 1.0f;
        this->fixscale.x = 1.0f;
    }
    if(mask & cxViewAutoResizeLeft){
        pos.x -= parent->size.w/2.0f;
        pos.x += size.w * this->anchor.x * scale.x;
        pos.x += this->autoBox.l;
    }
    if(mask & cxViewAutoResizeRight){
        pos.x += parent->size.w/2.0f;
        pos.x -= size.w * (1.0f - this->anchor.x) * scale.x;
        pos.x -= this->autoBox.r;
    }
    if((mask & cxViewAutoResizeTop) && (mask & cxViewAutoResizeBottom)){
        size.h = parent->size.h - this->autoBox.t - this->autoBox.b;
        this->scale.y = 1.0f;
        this->fixscale.y = 1.0f;
    }
    if(mask & cxViewAutoResizeTop){
        pos.y += parent->size.h/2.0f;
        pos.y -= size.h * (1.0f - this->anchor.y) * scale.y;
        pos.y -= this->autoBox.t;
    }
    if(mask & cxViewAutoResizeBottom){
        pos.y -= parent->size.h/2.0f;
        pos.y += size.h * this->anchor.y * scale.y;
        pos.y += this->autoBox.b;
    }
    cxViewSetPosition(this, pos);
    cxViewSetSize(this, size);
}

void cxViewLayout(cxAny pview)
{
    cxView this = pview;
    cxViewAutoResizing(this);
    CX_EVENT_FIRE(this, onLayout);
    CX_LIST_FOREACH(this->subViews, ele){
        cxView view = ele->object;
        cxViewLayout(view);
    }
}

//remove from parent
void cxViewRemoved(cxAny pview)
{
    cxView this = pview;
    CX_RETURN(this->parentView == NULL || this->subElement == NULL);
    if(this->isRunning){
        cxViewExit(this);
    }
    cxView parent = this->parentView;
    cxListRemove(parent->subViews, this->subElement);
    CX_EVENT_FIRE(parent, onChanged);
    this->subElement = NULL;
    this->parentView = NULL;
}

cxBool cxViewHitTest(cxAny pview,cxVec2f wPoint,cxVec2f *vPoint)
{
    cxVec2f pos = cxWindowPointToViewPoint(pview, wPoint);
    cxBox4f box = cxViewBox(pview);
    if(vPoint != NULL){
        *vPoint = pos;
    }
    return cxBox2fContainPoint(box, pos);
}

static cxBool cxViewTouchSubViews(cxAny pview,cxTouch *touch)
{
    cxView this = pview;
    cxListElement *head = cxListFirst(this->subViews);
    for(cxListElement *ele = cxListLast(this->subViews);ele != NULL && head != NULL;ele = ele->prev){
        cxView view = ele->object;
        if(cxViewTouch(view, touch)){
            return true;
        }
        if(ele == head){
            break;
        }
    }
    return false;
}

cxUInt cxViewIsTouch(cxAny pview,cxTouch *touch)
{
    cxView this = pview;
    if(!this->isVisible){
        return cxViewIsTouchTypeNone;
    }
    return cxViewIsTouchTypeSelf | cxViewIsTouchTypeSubview;
}

cxBool cxViewTouch(cxAny pview,cxTouch *touch)
{
    cxView this = pview;
    cxUInt type = CX_METHOD_GET(cxViewIsTouchTypeNone, this->IsTouch, this, touch);
    if(type == cxViewIsTouchTypeNone){
        return false;
    }
    if((type & cxViewIsTouchTypeSubview) && cxViewTouchSubViews(pview,touch)){
        return true;
    }
    if(type & cxViewIsTouchTypeSelf){
        return CX_METHOD_GET(false, this->Touch, this, touch);
    }
    return false;
}

cxBool cxViewHasAction(cxAny pview,cxUInt actionId)
{
    cxView this = pview;
    cxHashKey key = cxHashIntKey(actionId);
    return cxHashHas(this->actions, key);
}

void cxViewStopAction(cxAny pview,cxUInt actionId)
{
    cxView this = pview;
    cxHashKey key = cxHashIntKey(actionId);
    cxAny ptr = cxHashGet(this->actions, key);
    if(ptr != NULL){
        cxActionStop(ptr);
    }
}


cxUInt cxViewAppendAction(cxAny pview,cxAny pav)
{
    CX_ASSERT(pav != NULL, "action null");
    cxView this = pview;
    cxAction action = pav;
    action->view = pview;
    cxUInt actionId = cxActionGetId(action);
    cxHashKey key = cxHashIntKey(actionId);
    cxAny ptr = cxHashGet(this->actions, key);
    if(ptr != NULL){
        cxActionStop(ptr);
    }
    cxHashSet(this->actions, key, action);
    return actionId;
}

static void cxViewUpdateActions(cxView this)
{
    cxEngine engine = cxEngineInstance();
    CX_HASH_FOREACH(this->actions, ele, tmp){
        if(!cxActionUpdate(ele->any, engine->frameDelta)){
            continue;
        }
        cxHashDelElement(this->actions, ele);
    }
}

void cxViewDraw(cxAny pview)
{
    cxView this = pview;
    CX_EVENT_FIRE(this, onUpdate);
    CX_RETURN(!this->isVisible);
    //
    cxViewUpdateActions(this);
    cxViewTransform(this);
    //
    kmGLPushMatrix();
    kmGLMultMatrix(&this->normalMatrix);
    kmGLMultMatrix(&this->anchorMatrix);
    //
    if(this->isCropping){
        cxOpenGLEnableScissor(this->scissor);
    }
    CX_METHOD_RUN(this->DrawBefore,this);
    CX_METHOD_RUN(this->Draw,this);
    if(this->isSort){
        cxViewSort(this);
    }
    CX_LIST_SAFE_FOREACH(this->subViews, ele, tmp){
        cxView view = ele->object;
        cxViewDraw(view);
    }
    CX_METHOD_RUN(this->DrawAfter,this);
    if(this->isCropping){
        cxOpenGLDisableScissor();
    }
    cxViewDrawBorder(this);
    kmGLPopMatrix();
}









