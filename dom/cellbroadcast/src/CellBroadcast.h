/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_CellBroadcast_h__
#define mozilla_dom_CellBroadcast_h__

#include "nsDOMEventTargetHelper.h"
#include "nsIDOMMozCellBroadcast.h"
#include "nsICellBroadcastProvider.h"
#include "mozilla/Attributes.h"

class nsPIDOMWindow;

namespace mozilla {
namespace dom {

class CellBroadcast MOZ_FINAL : public nsDOMEventTargetHelper
                              , public nsIDOMMozCellBroadcast
                              , public nsICellBroadcastListener
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIDOMMOZCELLBROADCAST
  NS_DECL_NSICELLBROADCASTLISTENER

  NS_REALLY_FORWARD_NSIDOMEVENTTARGET(nsDOMEventTargetHelper)

  CellBroadcast() MOZ_DELETE;
  CellBroadcast(nsPIDOMWindow *aWindow,
                nsICellBroadcastProvider* aProvider);
  ~CellBroadcast();

private:
  nsCOMPtr<nsICellBroadcastProvider> mProvider;
};

} // namespace dom
} // namespace mozilla

nsresult
NS_NewCellBroadcast(nsPIDOMWindow* aWindow,
                    nsIDOMMozCellBroadcast** aCellBroadcast);

#endif // mozilla_dom_CellBroadcast_h__
