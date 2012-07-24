
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrClip.h"

GrClip::GrClip() 
    : fRequiresAA(false) {
    fConservativeBounds.setEmpty();
    fConservativeBoundsValid = true;
}

GrClip::GrClip(const GrClip& src) {
    *this = src;
}

GrClip::GrClip(const GrIRect& rect) {
    this->setFromIRect(rect);
}

GrClip::GrClip(const GrRect& rect) {
    this->setFromRect(rect);
}

GrClip::GrClip(GrClipIterator* iter, GrScalar tx, GrScalar ty,
               const GrRect& bounds) {
    this->setFromIterator(iter, tx, ty, bounds);
}

GrClip::~GrClip() {}

GrClip& GrClip::operator=(const GrClip& src) {
    fList = src.fList;
    fConservativeBounds = src.fConservativeBounds;
    fConservativeBoundsValid = src.fConservativeBoundsValid;
    fRequiresAA = src.fRequiresAA;
    return *this;
}

void GrClip::setEmpty() {
    fList.reset();
    fConservativeBounds.setEmpty();
    fConservativeBoundsValid = true;
    fRequiresAA = false;
}

void GrClip::setFromRect(const GrRect& r) {
    fList.reset();
    if (r.isEmpty()) {
        // use a canonical empty rect for == testing.
        setEmpty();
    } else {
        fList.push_back();
        fList.back().fRect = r;
        fList.back().fType = kRect_ClipType;
        fList.back().fOp = SkRegion::kReplace_Op;
        fList.back().fDoAA = false;
        fConservativeBounds = r;
        fConservativeBoundsValid = true;
        fRequiresAA = false;
    }
}

void GrClip::setFromIRect(const GrIRect& r) {
    fList.reset();
    if (r.isEmpty()) {
        // use a canonical empty rect for == testing.
        setEmpty();
    } else {
        fList.push_back();
        fList.back().fRect.set(r);
        fList.back().fType = kRect_ClipType;
        fList.back().fOp = SkRegion::kReplace_Op;
        fList.back().fDoAA = false;
        fConservativeBounds.set(r);
        fConservativeBoundsValid = true;
        fRequiresAA = false;
    }
}

static void intersectWith(SkRect* dst, const SkRect& src) {
    if (!dst->intersect(src)) {
        dst->setEmpty();
    }
}

void GrClip::setFromIterator(GrClipIterator* iter, GrScalar tx, GrScalar ty,
                             const GrRect& conservativeBounds) {
    fList.reset();
    fRequiresAA = false;

    int rectCount = 0;

    // compute bounds for common case of series of intersecting rects.
    bool isectRectValid = true;

    if (iter) {
        for (iter->rewind(); !iter->isDone(); iter->next()) {
            Element& e = fList.push_back();
            e.fType = iter->getType();
            e.fOp = iter->getOp();
            e.fDoAA = iter->getDoAA();
            if (e.fDoAA) {
                fRequiresAA = true;
            }
            // iterators should not emit replace
            GrAssert(SkRegion::kReplace_Op != e.fOp);
            switch (e.fType) {
                case kRect_ClipType:
                    iter->getRect(&e.fRect);
                    if (tx || ty) {
                        e.fRect.offset(tx, ty);
                    }
                    ++rectCount;
                    if (isectRectValid) {
                        if (SkRegion::kIntersect_Op == e.fOp) {
                            GrAssert(fList.count() <= 2);
                            if (fList.count() > 1) {
                                GrAssert(2 == rectCount);
                                rectCount = 1;
                                fList.pop_back();
                                GrAssert(kRect_ClipType == fList.back().fType);
                                intersectWith(&fList.back().fRect, e.fRect);
                            }
                        } else {
                            isectRectValid = false;
                        }
                    }
                    break;
                case kPath_ClipType:
                    e.fPath = *iter->getPath();
                    if (tx || ty) {
                        e.fPath.offset(tx, ty);
                    }
                    e.fPathFill = iter->getPathFill();
                    isectRectValid = false;
                    break;
                default:
                    GrCrash("Unknown clip element type.");
            }
        }
    }
    fConservativeBoundsValid = false;
    if (isectRectValid && rectCount) {
        fConservativeBounds = fList[0].fRect;
        fConservativeBoundsValid = true;
    } else {
        fConservativeBounds = conservativeBounds;
        fConservativeBoundsValid = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

GrClip::Iter::Iter() 
    : fStack(NULL)
    , fCurIndex(0) {
}

GrClip::Iter::Iter(const GrClip& stack, IterStart startLoc)
    : fStack(&stack) {
    this->reset(stack, startLoc);
}

const GrClip::Iter::Clip* GrClip::Iter::updateClip(int index) {

    if (NULL == fStack) {
        return NULL;
    }

    GrAssert(0 <= index && index < fStack->getElementCount());



    switch (fStack->getElementType(index)) {
        case kRect_ClipType:
            fClip.fRect = &fStack->getRect(index);
            fClip.fPath = NULL;
            break;
        case kPath_ClipType:
            fClip.fRect = NULL;
            fClip.fPath = &fStack->getPath(index);
            break;
    }
    fClip.fOp = fStack->getOp(index);
    fClip.fDoAA = fStack->getDoAA(index);
    return &fClip;
}

const GrClip::Iter::Clip* GrClip::Iter::next() {

    if (NULL == fStack) {
        return NULL;
    }

    if (0 > fCurIndex || fCurIndex >= fStack->getElementCount()) {
        return NULL;
    }

    int oldIndex = fCurIndex;
    ++fCurIndex;

    return this->updateClip(oldIndex);
}

const GrClip::Iter::Clip* GrClip::Iter::prev() {

    if (NULL == fStack) {
        return NULL;
    }

    if (0 > fCurIndex || fCurIndex >= fStack->getElementCount()) {
        return NULL;
    }

    int oldIndex = fCurIndex;
    --fCurIndex;

    return this->updateClip(oldIndex);
}

const GrClip::Iter::Clip* GrClip::Iter::skipToTopmost(SkRegion::Op op) {

    GrAssert(SkRegion::kReplace_Op == op);

    if (NULL == fStack) {
        return NULL;
    }

    // GrClip removes all clips below the topmost replace
    this->reset(*fStack, kBottom_IterStart);

    return this->next();
}

void GrClip::Iter::reset(const GrClip& stack, IterStart startLoc) {
    fStack = &stack;
    if (kBottom_IterStart == startLoc) {
        fCurIndex = 0;
    } else {
        fCurIndex = fStack->getElementCount()-1;
    }
}
