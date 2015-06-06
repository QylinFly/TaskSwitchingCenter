#include "icommand.h"

using namespace PackageQyLinCommon;

ICommand::ICommand()
{
    m_ProcessType = EnumProcessType::eSerial;
    m_ComType = -1;
	ID = 0;
}

ICommand::~ICommand(){}
