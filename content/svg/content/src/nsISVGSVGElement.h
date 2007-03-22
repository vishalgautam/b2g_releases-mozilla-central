/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is
 * Crocodile Clips Ltd..
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alex Fritze <alex.fritze@crocodile-clips.com> (original author)
 *   Jonathan Watt <jonathan.watt@strath.ac.uk>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __NS_ISVGSVGELEMENT__
#define __NS_ISVGSVGELEMENT__

#include "nsISupports.h"
#include "nsIDOMSVGSVGElement.h"

class nsSVGSVGElement;
class nsIDOMSVGNumber;
class nsISVGEnum;

////////////////////////////////////////////////////////////////////////
// nsISVGSVGElement: private interface implemented by <svg>-elements

#define NS_ISVGSVGELEMENT_IID \
{ 0x65195064, 0x9a5f, 0x42df, { 0xa1, 0x1f, 0x4c, 0x05, 0xbc, 0xa4, 0xe7, 0x28 } }

class nsISVGSVGElement : public nsIDOMSVGSVGElement
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ISVGSVGELEMENT_IID)

  NS_IMETHOD GetCurrentScaleNumber(nsIDOMSVGNumber **aResult)=0;
  NS_IMETHOD GetZoomAndPanEnum(nsISVGEnum **aResult)=0;

  /**
   * For use by zoom controls to allow currentScale, currentTranslate.x and
   * currentTranslate.y to be set by a single operation that dispatches a
   * single SVGZoom event (instead of one SVGZoom and two SVGScroll events).
   */
  NS_IMETHOD SetCurrentScaleTranslate(float s, float x, float y)=0;

  /**
   * For use by pan controls to allow currentTranslate.x and currentTranslate.y
   * to be set by a single operation that dispatches a single SVGScroll event
   * (instead of two).
   */
  NS_IMETHOD SetCurrentTranslate(float x, float y)=0;

  /**
   * Record the current values of currentScale, currentTranslate.x and
   * currentTranslate.y prior to changing the value of one of them.
   */
  NS_IMETHOD_(void) RecordCurrentScaleTranslate()=0;

  /**
   * Retrieve the value of currentScale, currentTranslate.x or
   * currentTranslate.y prior to the last change made to any one of them.
   */
  NS_IMETHOD_(float) GetPreviousScale()=0;
  NS_IMETHOD_(float) GetPreviousTranslate_x()=0;
  NS_IMETHOD_(float) GetPreviousTranslate_y()=0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsISVGSVGElement, NS_ISVGSVGELEMENT_IID)

#endif // __NS_ISVGSVGELEMENT__
