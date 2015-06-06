#ifndef TASKSWITCHINGCENTER_H
#define TASKSWITCHINGCENTER_H
//#include "../kylincommmon_global.h"
//#include "../three/osgInc.h"
#include "icommand.h"

#include <QtCore>
#include <QThread>
#include <QReadWriteLock>
#include <QWaitCondition>
#include <QSemaphore>
#include <QFutureWatcher>
#include <list>
#include <string>

#include <QVector>
#include <QMap>

/**
* @brief The TaskSwitchingCenter class
* 任务交换中心
* 1.完成任务的分类接受（分类指任务类型分类和处理方式分类）
* 2.完成任务类型和指定处理方式的处理
* 处理方式分为:1.串行 2.并行 串行放入各自指定的串行队列 并行放入统一的并行队列（）
* 任务类型分为：1.普通 2.
*/
#define KYLINCOMMONSHARED_EXPORT
//#define //g_loger //
#define  LOGINOUT

namespace PackageQyLinCommon
{

class KYLINCOMMONSHARED_EXPORT ICommandDoubleBuffers/*:public QObject*/
{
	/*Q_OBJECT*/
public:
	ICommandDoubleBuffers();
public:
	//处理函数
	virtual void doProcess () = 0;
	//数据插入
	virtual bool doInsert(ICommand *command);
	//队列交换
	void swapInsertlist();
protected:
	QList<ICommand*> m_listCommand[2];
	bool m_bInsertlist;//当前插入 m_bInsertlist为插入列表 !m_bInsertlist为非插入列表
	QReadWriteLock m_readWriteLock;
protected:
	QMutex m_InsertMutex;

};

class KYLINCOMMONSHARED_EXPORT CommandDoubleBuffers:public ICommandDoubleBuffers
{
public:
    CommandDoubleBuffers(EnumProcessType parProcessType = EnumProcessType(eSerial));
	//处理
	virtual void doProcess ();
	//插入
	bool doInsert(ICommand* command);
	//串行处理
	void doSerialProcess ();
	//并行处理
	void doParallelProcess();

protected:
	EnumProcessType m_ProcessType;
	QSemaphore m_semaphore;
	QFutureWatcher<void> *watcherDoSerial;
};

class KYLINCOMMONSHARED_EXPORT CommandSerial :public ICommandDoubleBuffers
{
public:
	CommandSerial(){};
	QWaitCondition m_waitCondition;
	
	virtual bool doInsert(ICommand *command);
	virtual void doProcess ();
};

class KYLINCOMMONSHARED_EXPORT CommandParallel :public ICommandDoubleBuffers
{
public:
	CommandParallel(){};
	QWaitCondition m_waitCondition;
	virtual bool doInsert(ICommand *command);
	virtual void doProcess ();
};

class KYLINCOMMONSHARED_EXPORT CommandSerialThread :public QThread,public CommandSerial
{
	Q_OBJECT
public:
	CommandSerialThread(){};
	virtual void run (){	doProcess();	}
};

class KYLINCOMMONSHARED_EXPORT CommandParallelThread :public QThread,public CommandParallel
{
	Q_OBJECT
public:
	CommandParallelThread(){};
	virtual void run (){ doProcess();}
};

class KYLINCOMMONSHARED_EXPORT TaskSwitchingCenter :public QThread,public ICommandDoubleBuffers
{
	Q_OBJECT
public:
    TaskSwitchingCenter( EnumProcessType parProcessType = EnumProcessType(eSerial));
    ~TaskSwitchingCenter();

public:
    //
    int createProThread(std::string name,CommandDoubleBuffers *parCommandDoubleBuffers = NULL,
                        EnumProcessType parThreadProcessType = EnumProcessType(eSerial));

	//向默认串行队列插入命令
	void doInsertDefaultSerial(ICommand* command);
	//向默认并行队列插入命令
	void doInsertDefaultParallel(ICommand* command);

    //给定队列ID插入命令
	void doInsertDirect(ICommand* command,int ID);

	//重载 ICommandDoubleBuffers::doInsert( ICommand *command )
	//去除在command->DoPre();在该函数执行
    //insert command to dublebuffer,
    virtual bool doInsert(ICommand *command);
    //insert command to ThreadBuffer function
    virtual void funDoInsert(ICommand* command);
    //
    virtual bool doConcurrent();
	virtual bool doSerial();

private:
    void doProcess ();

    virtual void run ();

protected:
	EnumProcessType m_ProcessType;//处理方式
	QVector<ICommandDoubleBuffers*> ThreadBuffer;
	QWaitCondition m_waitCondition;


    QFutureWatcher<void> *watcherDoConcurrent;

	//QSemaphore m_semaphoreC;

	QMutex m_InsertThreadBufferMutex; 
	QMap<ICommandDoubleBuffers*,ICommandDoubleBuffers*> m_tempThreadBuffer;
	

};
}
#endif // TASKSWITCHINGCENTER_H
