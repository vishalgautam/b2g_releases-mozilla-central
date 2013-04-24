/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CellBroadcastService.h"

using namespace mozilla::dom::cellbroadcast;

NS_IMPL_ISUPPORTS1(CellBroadcastService,
                   nsICellBroadcastProvider);

NS_IMETHODIMP
CellBroadcastService::Register(nsICellBroadcastListener* aListener)
{
  NS_ENSURE_STATE(!mListeners.Contains(aListener));

  return mListeners.AppendElement(aListener) ? NS_OK : NS_ERROR_FAILURE;
}

NS_IMETHODIMP
CellBroadcastService::Unregister(nsICellBroadcastListener* aListener)
{
  return mListeners.RemoveElement(aListener) ? NS_OK : NS_ERROR_FAILURE;
}
