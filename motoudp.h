// Copyright (c) 2019 Sagar Gubbi. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOTOUDP_H
#define MOTOUDP_H

int mpUDPInit(unsigned int port, void (*udpCallback)(char* buf, int buf_len));
int mpUDPSend(char *buf, int buf_len);
int mpUDPPrintf(const char* fmt, ...);

#endif
