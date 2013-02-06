/*
	cain-sip - SIP (RFC3261) library.
    Copyright (C) 2010-2013  Belledonne Communications SARL

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CAIN_SIP_DEFS_H
#define CAIN_SIP_DEFS_H

#ifdef __cplusplus
#define CAIN_SIP_BEGIN_DECLS		extern "C"{
#define CAIN_SIP_END_DECLS		}
#else
#define CAIN_SIP_BEGIN_DECLS
#define CAIN_SIP_END_DECLS
#endif

#ifdef _MSC_VER
#define inline __inline
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)
#ifdef CAINSIP_EXPORTS
#define CAINSIP_EXPORT __declspec(dllexport)
#else
#define CAINSIP_EXPORT extern
#endif
#else
#define CAINSIP_EXPORT extern
#endif

#ifdef _MSC_VER
CAINSIP_EXPORT int gettimeofday (struct timeval *tv, void* tz);
#endif

#endif
