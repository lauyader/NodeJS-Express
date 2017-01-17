/*
  Firmata.cpp - Firmata library v2.5.4 - 2016-10-23
  Copyright (c) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2009-2016 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

//******************************************************************************
//* Includes
//******************************************************************************

#include "FirmataParser.h"

#include "FirmataConstants.h"

//******************************************************************************
//* Constructors
//******************************************************************************

/**
 * The FirmataParser class.
 * @param dataBuffer A pointer to an external buffer used to store parsed data
 * @param dataBufferSize The size of the external buffer
 */
FirmataParser::FirmataParser(uint8_t * const dataBuffer, size_t dataBufferSize)
:
  dataBuffer(dataBuffer),
  dataBufferSize(dataBufferSize),
  executeMultiByteCommand(0),
  multiByteChannel(0),
  waitForData(0),
  parsingSysex(false),
  sysexBytesRead(0),
  currentDataBufferOverflowCallbackContext((void *)NULL),
  currentAnalogCallback((callbackFunction)NULL),
  currentDigitalCallback((callbackFunction)NULL),
  currentReportAnalogCallback((callbackFunction)NULL),
  currentReportDigitalCallback((callbackFunction)NULL),
  currentPinModeCallback((callbackFunction)NULL),
  currentPinValueCallback((callbackFunction)NULL),
  currentReportFirmwareCallback((systemCallbackFunction)NULL),
  currentReportVersionCallback((systemCallbackFunction)NULL),
  currentSystemResetCallback((systemCallbackFunction)NULL),
  currentStringCallback((stringCallbackFunction)NULL),
  currentSysexCallback((sysexCallbackFunction)NULL),
  currentDataBufferOverflowCallback((dataBufferOverflowCallbackFunction)NULL)
{
    allowBufferUpdate = ((uint8_t *)NULL == dataBuffer);
}

//******************************************************************************
//* Public Methods
//******************************************************************************

//------------------------------------------------------------------------------
// Serial Receive Handling

/**
 * Parse data from the input stream.
 * @param inputData A single byte to be added to the parser.
 */
void FirmataParser::parse(uint8_t inputData)
{
  uint8_t command;

  if (parsingSysex) {
    if (inputData == END_SYSEX) {
      //stop sysex byte
      parsingSysex = false;
      //fire off handler function
      processSysexMessage();
    } else {
      //normal data byte - add to buffer
      bufferDataAtPosition(inputData, sysexBytesRead);
      ++sysexBytesRead;
    }
  } else if ( (waitForData > 0) && (inputData < 128) ) {
    --waitForData;
    bufferDataAtPosition(inputData, waitForData);
    if ( (waitForData == 0) && executeMultiByteCommand ) { // got the whole message
      switch (executeMultiByteCommand) {
        case ANALOG_MESSAGE:
          if (currentAnalogCallback) {
            (*currentAnalogCallback)(multiByteChannel,
                                     (dataBuffer[0] << 7)
                                     + dataBuffer[1]);
          }
          break;
        case DIGITAL_MESSAGE:
          if (currentDigitalCallback) {
            (*currentDigitalCallback)(multiByteChannel,
                                      (dataBuffer[0] << 7)
                                      + dataBuffer[1]);
          }
          break;
        case SET_PIN_MODE:
          if (currentPinModeCallback)
            (*currentPinModeCallback)(dataBuffer[1], dataBuffer[0]);
          break;
        case SET_DIGITAL_PIN_VALUE:
          if (currentPinValueCallback)
            (*currentPinValueCallback)(dataBuffer[1], dataBuffer[0]);
          break;
        case REPORT_ANALOG:
          if (currentReportAnalogCallback)
            (*currentReportAnalogCallback)(multiByteChannel, dataBuffer[0]);
          break;
        case REPORT_DIGITAL:
          if (currentReportDigitalCallback)
            (*currentReportDigitalCallback)(multiByteChannel, dataBuffer[0]);
          break;
      }
      executeMultiByteCommand = 0;
    }
  } else {
    // remove channel info from command byte if less than 0xF0
    if (inputData < 0xF0) {
      command = inputData & 0xF0;
      multiByteChannel = inputData & 0x0F;
    } else {
      command = inputData;
      // commands in the 0xF* range don't use channel data
    }
    switch (command) {
      case ANALOG_MESSAGE:
      case DIGITAL_MESSAGE:
      case SET_PIN_MODE:
      case SET_DIGITAL_PIN_VALUE:
        waitForData = 2; // two data bytes needed
        executeMultiByteCommand = command;
        break;
      case REPORT_ANALOG:
      case REPORT_DIGITAL:
        waitForData = 1; // one data byte needed
        executeMultiByteCommand = command;
        break;
      case START_SYSEX:
        parsingSysex = true;
        sysexBytesRead = 0;
        break;
      case SYSTEM_RESET:
        systemReset();
        break;
      case REPORT_VERSION:
        if (currentReportVersionCallback)
          (*currentReportVersionCallback)();
        break;
    }
  }
}

/**
 * @return Returns true if the parser is actively parsing data.
 */
bool FirmataParser::isParsingMessage(void)
const
{
  return (waitForData > 0 || parsingSysex);
}

/**
 * Provides a mechanism to either set or update the working buffer of the parser.
 * The method will be enabled when no buffer has been provided, or an overflow
 * condition exists.
 * @param dataBuffer A pointer to an external buffer used to store parsed data
 * @param dataBufferSize The size of the external buffer
 */
int FirmataParser::setDataBufferOfSize(uint8_t * dataBuffer, size_t dataBufferSize)
{
    int result;

    if ( !allowBufferUpdate ) {
      result = __LINE__;
    } else if ((uint8_t *)NULL == dataBuffer) {
      result = __LINE__;
    } else {
      this->dataBuffer = dataBuffer;
      this->dataBufferSize = dataBufferSize;
      allowBufferUpdate = false;
      result = 0;
    }

    return result;
}

/**
 * Attach a generic sysex callback function to a command (options are: ANALOG_MESSAGE,
 * DIGITAL_MESSAGE, REPORT_ANALOG, REPORT DIGITAL, SET_PIN_MODE and SET_DIGITAL_PIN_VALUE).
 * @param command The ID of the command to attach a callback function to.
 * @param newFunction A reference to the callback function to attach.
 */
void FirmataParser::attach(uint8_t command, callbackFunction newFunction)
{
  switch (command) {
    case ANALOG_MESSAGE: currentAnalogCallback = newFunction; break;
    case DIGITAL_MESSAGE: currentDigitalCallback = newFunction; break;
    case REPORT_ANALOG: currentReportAnalogCallback = newFunction; break;
    case REPORT_DIGITAL: currentReportDigitalCallback = newFunction; break;
    case SET_PIN_MODE: currentPinModeCallback = newFunction; break;
    case SET_DIGITAL_PIN_VALUE: currentPinValueCallback = newFunction; break;
  }
}

/**
 * Attach a system callback function (options are: REPORT_FIRMWARE, REPORT_VERSION
 * and SYSTEM_RESET).
 * @param command The ID of the command to attach a callback function to.
 * @param newFunction A reference to the callback function to attach.
 */
void FirmataParser::attach(uint8_t command, systemCallbackFunction newFunction)
{
  switch (command) {
    case REPORT_FIRMWARE: currentReportFirmwareCallback = newFunction; break;
    case REPORT_VERSION: currentReportVersionCallback = newFunction; break;
    case SYSTEM_RESET: currentSystemResetCallback = newFunction; break;
  }
}

/**
 * Attach a callback function for the STRING_DATA command.
 * @param command Must be set to STRING_DATA or it will be ignored.
 * @param newFunction A reference to the string callback function to attach.
 */
void FirmataParser::attach(uint8_t command, stringCallbackFunction newFunction)
{
  switch (command) {
    case STRING_DATA: currentStringCallback = newFunction; break;
  }
}

/**
 * Attach a generic sysex callback function to sysex command.
 * @param command The ID of the command to attach a callback function to.
 * @param newFunction A reference to the sysex callback function to attach.
 */
void FirmataParser::attach(uint8_t command, sysexCallbackFunction newFunction)
{
  (void)command;
  currentSysexCallback = newFunction;
}

/**
 * Attach a buffer overflow callback
 * @param newFunction A reference to the buffer overflow callback function to attach.
 * @param context The context supplied by the end-user, and provided during the execution of the callback
 */
void FirmataParser::attach(dataBufferOverflowCallbackFunction newFunction, void * context)
{
  currentDataBufferOverflowCallback = newFunction;
  currentDataBufferOverflowCallbackContext = context;
}

/**
 * Detach a callback function for a specified command (such as SYSTEM_RESET, STRING_DATA,
 * ANALOG_MESSAGE, DIGITAL_MESSAGE, etc).
 * @param command The ID of the command to detatch the callback function from.
 */
void FirmataParser::detach(uint8_t command)
{
  switch (command) {
    case REPORT_FIRMWARE:
    case REPORT_VERSION:
    case SYSTEM_RESET:
      attach(command, (systemCallbackFunction)NULL);
      break;
    case STRING_DATA: currentStringCallback = (stringCallbackFunction)NULL; break;
    case START_SYSEX: currentSysexCallback = (sysexCallbackFunction)NULL; break;
    default:
      attach(command, (callbackFunction)NULL);
  }
}

/**
 * Detach the buffer overflow callback
 * @param <unused> Any pointer of type dataBufferOverflowCallbackFunction.
 */
void FirmataParser::detach(dataBufferOverflowCallbackFunction)
{
  currentDataBufferOverflowCallback = (dataBufferOverflowCallbackFunction)NULL;
  currentDataBufferOverflowCallbackContext = (void *)NULL;
}

//******************************************************************************
//* Private Methods
//******************************************************************************

/**
 * Buffer abstraction to prevent memory corruption
 * @param data The byte to put into the buffer
 * @param pos The position to insert the byte into the buffer
 * @return writeError A boolean to indicate if an error occured
 * @private
 */
bool FirmataParser::bufferDataAtPosition(const uint8_t data, const size_t pos)
{
  bool bufferOverflow = (pos >= dataBufferSize);

  // Notify of overflow condition
  if ( bufferOverflow
  && ((dataBufferOverflowCallbackFunction)NULL != currentDataBufferOverflowCallback) )
  {
    allowBufferUpdate = true;
    currentDataBufferOverflowCallback(currentDataBufferOverflowCallbackContext);
    // Check if overflow was resolved during callback
    bufferOverflow = (pos >= dataBufferSize);
  }

  // Write data to buffer if no overflow condition persist
  if ( !bufferOverflow )
  {
    dataBuffer[pos] = data;
  }

  return bufferOverflow;
}

/**
 * Process incoming sysex messages. Handles REPORT_FIRMWARE and STRING_DATA internally.
 * Calls callback function for STRING_DATA and all other sysex messages.
 * @private
 */
void FirmataParser::processSysexMessage(void)
{
  switch (dataBuffer[0]) { //first byte in buffer is command
    case REPORT_FIRMWARE:
      if (currentReportFirmwareCallback)
        (*currentReportFirmwareCallback)();
      break;
    case STRING_DATA:
      if (currentStringCallback) {
        size_t bufferLength = (sysexBytesRead - 1) / 2;
        size_t i = 1;
        size_t j = 0;
        while (j < bufferLength) {
          // The string length will only be at most half the size of the
          // stored input buffer so we can decode the string within the buffer.
          bufferDataAtPosition(dataBuffer[i], j);
          ++i;
          bufferDataAtPosition((dataBuffer[j] + (dataBuffer[i] << 7)), j);
          ++i;
          ++j;
        }
        // Make sure string is null terminated. This may be the case for data
        // coming from client libraries in languages that don't null terminate
        // strings.
        if (dataBuffer[j - 1] != '\0') {
          bufferDataAtPosition('\0', j);
        }
        (*currentStringCallback)((char *)&dataBuffer[0]);
      }
      break;
    default:
      if (currentSysexCallback)
        (*currentSysexCallback)(dataBuffer[0], sysexBytesRead - 1, dataBuffer + 1);
  }
}

/**
 * Resets the system state upon a SYSTEM_RESET message from the host software.
 * @private
 */
void FirmataParser::systemReset(void)
{
  size_t i;

  waitForData = 0; // this flag says the next serial input will be data
  executeMultiByteCommand = 0; // execute this after getting multi-byte data
  multiByteChannel = 0; // channel data for multiByteCommands

  for (i = 0; i < dataBufferSize; ++i) {
    dataBuffer[i] = 0;
  }

  parsingSysex = false;
  sysexBytesRead = 0;

  if (currentSystemResetCallback)
    (*currentSystemResetCallback)();
}