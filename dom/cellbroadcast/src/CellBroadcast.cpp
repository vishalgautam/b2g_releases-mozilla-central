/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CellBroadcast.h"
#include "nsIDOMMozCellBroadcastEvent.h"
#include "nsIDOMMozCellBroadcastMessage.h"
#include "mozilla/Services.h"
#include "nsDOMClassInfo.h"
#include "GeneratedEvents.h"

using namespace mozilla::dom;

/**
 * CellBroadcast Implementation.
 */

DOMCI_DATA(MozCellBroadcast, CellBroadcast)

NS_INTERFACE_MAP_BEGIN(CellBroadcast)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMozCellBroadcast)
  NS_INTERFACE_MAP_ENTRY(nsICellBroadcastListener)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(MozCellBroadcast)
NS_INTERFACE_MAP_END_INHERITING(nsDOMEventTargetHelper)

NS_IMPL_ADDREF_INHERITED(CellBroadcast, nsDOMEventTargetHelper)
NS_IMPL_RELEASE_INHERITED(CellBroadcast, nsDOMEventTargetHelper)

CellBroadcast::CellBroadcast(nsPIDOMWindow *aWindow,
                             nsICellBroadcastProvider *aProvider)
  : mProvider(aProvider)
{
  BindToOwner(aWindow);

  DebugOnly<nsresult> rv = mProvider->Register(this);
  NS_WARN_IF_FALSE(NS_SUCCEEDED(rv),
                   "Failed registering Cell Broadcast callback with provider");
}

CellBroadcast::~CellBroadcast()
{
  MOZ_ASSERT(mProvider);

  mProvider->Unregister(this);
}

NS_IMPL_EVENT_HANDLER(CellBroadcast, received)

// Forwarded nsICellBroadcastListener methods

NS_IMETHODIMP
CellBroadcast::NotifyMessageReceived(nsIDOMMozCellBroadcastMessage* aMessage)
{
  nsCOMPtr<nsIDOMEvent> event;
  NS_NewDOMMozCellBroadcastEvent(getter_AddRefs(event), this, nullptr, nullptr);

  nsCOMPtr<nsIDOMMozCellBroadcastEvent> ce = do_QueryInterface(event);
  nsresult rv = ce->InitMozCellBroadcastEvent(NS_LITERAL_STRING("received"),
                                              true, false, aMessage);
  NS_ENSURE_SUCCESS(rv, rv);

  return DispatchTrustedEvent(ce);
}

nsresult
NS_NewCellBroadcast(nsPIDOMWindow* aWindow,
                    nsIDOMMozCellBroadcast** aCellBroadcast)
{
  nsPIDOMWindow* innerWindow = aWindow->IsInnerWindow() ?
    aWindow :
    aWindow->GetCurrentInnerWindow();

  nsCOMPtr<nsICellBroadcastProvider> provider =
    do_GetService(CELLBROADCAST_SERVICE_CONTRACTID);
  NS_ENSURE_STATE(provider);

  nsRefPtr<mozilla::dom::CellBroadcast> cb =
    new mozilla::dom::CellBroadcast(innerWindow, provider);
  cb.forget(aCellBroadcast);

  return NS_OK;
}
