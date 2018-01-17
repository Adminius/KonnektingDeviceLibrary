/*
 *    This file is part of KONNEKTING Knx Device Library.
 * 
 *    It is derived from another GPLv3 licensed project:
 *      The Arduino Knx Bus Device library allows to turn Arduino into "self-made" KNX bus device.
 *      Copyright (C) 2014 2015 Franck MARINI (fm@liwan.fr)
 *
 *    The KONNEKTING Knx Device Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


// File : KnxComObject.cpp
// Author : Franck Marini
// Modified: Alexander Christian <info(at)root1.de>
// Description : Handling of the KNX Communication Objects
// Module dependencies : KnxTelegram

#include <DebugUtil.h>
#include "KnxComObject.h"

// Data length is calculated in the same way as telegram payload length

byte calcLength(e_KnxDPT_ID dptId) {
    return (pgm_read_byte(&KnxDPTFormatToLengthBit[ pgm_read_byte(&KnxDPTIdToFormat[dptId])]) / 8) + 1;
}

/**
 * Contructor
 * @param dptId
 * @param indicator
 */
KnxComObject::KnxComObject(e_KnxDPT_ID dptId, byte indicator)
: _dptId(dptId), _indicator(indicator), _length(calcLength(dptId)) {
    _active = false;
    if (_length <= 2) {
        _longValue = NULL; // short value case
    } else { // long value case
        _longValue = (byte *) malloc(_length - 1);
        for (byte i = 0; i < _length - 1; i++) _longValue[i] = 0;
    }
    if (_indicator & KNX_COM_OBJ_I_INDICATOR) {
        _validity = false; // case of object with "InitRead" indicator
    } else {
        _validity = true; // case of object without "InitRead" indicator
    }
}

/**
 * Destructor
 */
KnxComObject::~KnxComObject() {
    if (_length > 2) free(_longValue);
}

/**
 * TODO document me
 * @return 
 */
bool KnxComObject::isActive() {
    return _active;
}

/**
 * TODO document me
 * @param flag
 */
void KnxComObject::setActive(bool flag) {
    _active = flag;
}


// Get the com obj value (short and long value cases)

void KnxComObject::getValue(byte dest[]) const {
    if (_length <= 2) {
        dest[0] = _value; // short value case, ReadValue(void) fct should rather be used
    } else {
        for (byte i = 0; i < _length - 1; i++) {
            dest[i] = _longValue[i]; // long value case
        }
    }
}

// Update the com obj value (short and long value cases)

void KnxComObject::updateValue(const byte ori[]) {
    if (_length <= 2) {
        _value = ori[0]; // short value case, UpdateValue(byte) fct should rather be used
    } else {
        for (byte i = 0; i < _length - 1; i++) {
            _longValue[i] = ori[i]; // long value case
            //DEBUG_PRINTLN(F("_longValue[%d]=0x%02x == ori[%d]=0x%02x"), i, _longValue[i], i, ori[i]);
        }
    }
    _validity = true; // com obj set to valid
}


// Update the com obj value with the telegram payload content

byte KnxComObject::updateValue(const KnxTelegram& ori) {
    if (ori.GetPayloadLength() != getLength()) return KNX_COM_OBJECT_ERROR; // Error : telegram payload length differs from com obj one
    if (_length == 1) _value = ori.GetFirstPayloadByte();
    else if (_length == 2) ori.GetLongPayload(&_value, 1);
    else ori.GetLongPayload(_longValue, _length - 1);
    _validity = true; // com object set to valid
    return KNX_COM_OBJECT_OK;
}


// Copy the com obj attributes (addr, prio & length) into a telegram object

void KnxComObject::copyAttributes(KnxTelegram& dest) const {
    dest.ChangePriority(getPriority());
    dest.SetTargetAddress(getAddr());
    dest.SetPayloadLength(_length); // case short length
}


// Copy the com obj value into a telegram object

void KnxComObject::copyValue(KnxTelegram& dest) const {
    if (_length == 1) dest.SetFirstPayloadByte(_value);
    else if (_length == 2)dest.SetLongPayload(&_value, 1);
    else dest.SetLongPayload(_longValue, _length - 1);
}