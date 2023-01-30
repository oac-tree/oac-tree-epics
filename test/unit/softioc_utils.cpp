/******************************************************************************
 *
 * Project       : Supervision and automation system EPICS interface
 *
 * Description   : Library of SUP components for EPICS network protocol
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2022 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 *****************************************************************************/

#include "softioc_utils.h"

#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>

namespace
{
const std::string db_content = R"RAW(
record (bo,"SEQ-TEST:BOOL")
{
    field(DESC,"Some EPICSv3 record")
    field(ONAM,"TRUE")
    field(OSV,"NO_ALARM")
    field(ZNAM,"FALSE")
    field(ZSV,"NO_ALARM")
    field(VAL,"0")
    field(PINI, "YES")
}

record (ao,"SEQ-TEST:FLOAT")
{
    field(DESC,"Some EPICSv3 record")
    field(DRVH,"5.0")
    field(DRVL,"-5.0")
    field(VAL,"0")
    field(PINI, "YES")
}

record (stringout,"SEQ-TEST:STRING")
{
    field(DESC,"Some EPICSv3 record")
    field(PINI, "YES")
}

record (waveform,"SEQ-TEST:UIARRAY")
{
    field(DESC,"Some EPICSv3 record")
    field(FTVL, "ULONG")
    field(NELM, "8")
    field(PINI, "YES")
}

record (waveform,"SEQ-TEST:CHARRAY")
{
    field(DESC,"Some EPICSv3 record")
    field(FTVL, "CHAR")
    field(NELM, "1024")
    field(PINI, "YES")
}
)RAW";

}  // unnamed namespace

namespace sup {

namespace sequencer {

namespace softioc_utils {

std::string GetEPICSBinaryPath()
{
  return std::string(std::getenv("EPICS_BASE")) + "/bin/"
         + std::string(std::getenv("EPICS_HOST_ARCH")) + "/";
}

std::string GetEpicsDBContentString()
{
  return db_content;
}

void RemoveFile(const std::string &file_name)
{
  std::remove(file_name.c_str());
}

} // namespace softioc_utils

} // namespace sequencer

} // namespace sup
