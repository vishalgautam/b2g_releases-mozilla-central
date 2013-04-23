/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CellBroadcastChild.h"

using namespace mozilla::dom::cellbroadcast;

bool
CellBroadcastChild::RecvNotifyMessageReceived(const MessageData& aMessageData)
{
  // FIXME: mListener->NotifyMessageReceived(message);
  return true;
}
