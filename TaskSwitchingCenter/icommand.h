#ifndef ICOMMAND_H
#define ICOMMAND_H
#include <string>
//#include "../kylincommmon_global.h"
#define  KYLINCOMMONSHARED_EXPORT

namespace PackageQyLinCommon
{
 enum EnumProcessType{ eSerial, eParallel};

class KYLINCOMMONSHARED_EXPORT ICommand
{
public:
    ICommand();
    virtual ~ICommand();
public:
	virtual void DoPre() = 0;//在执行Do前尽可能要执行的操作 该操作在双缓冲链表插入时执行
    virtual void Do() = 0;
    virtual void UnDo() = 0;
public:
   int ID;//内部使用
    EnumProcessType m_ProcessType;//处理方式
    int m_ComType;//命令类型
	std::string m_strCmdName;
};
}
#endif // ICOMMAND_H
