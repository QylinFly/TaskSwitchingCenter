#include "taskswitchingcenter.h"
#include <qtconcurrentmap.h>
#include <qtconcurrentmedian.h>

#include <QImage>
#include <QList>
#include <QThread>
#include <QDebug>
#include <QApplication>
#include <qtconcurrentmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

//#include "../LOG4CPP/Log.h"

using namespace PackageQyLinCommon;

void FunMapConcurrent(ICommandDoubleBuffers* p)
{
//    QPointer<QObject> label = dynamic_cast<QObject*>(p);
//    if ( !label.isNull() )
	{
		//g_loger.debug("in FunMapConcurrent&---%d--%d",	p	);
		p->doProcess();
        //g_loger.debug("out FunMapConcurrent&---%d--",	p	);
	}
	return;
}
int FunMapCommand( ICommand *command)
{
	if (command)
	{
		command->Do();
		//command->Do();
		delete command;
		command = NULL;
	}
    return 1;
}
TaskSwitchingCenter::TaskSwitchingCenter( EnumProcessType parProcessType /*= EnumProcessType::eSerial*/ )
{
    //g_loger.debug("in TaskSwitchingCenter::TaskSwitchingCenter");
    m_ProcessType = parProcessType;
    watcherDoConcurrent = new QFutureWatcher<void>();

    CommandParallelThread * p = new CommandParallelThread();
    ThreadBuffer.insert(0,p);
	//p->setPriority(QThread::LowestPriority);
    p->start(QThread::IdlePriority);

    CommandSerialThread * pp = new CommandSerialThread();
    ThreadBuffer.insert(1,pp);
	//pp->setPriority(QThread::LowestPriority);
    pp->start(QThread::IdlePriority);
}

TaskSwitchingCenter::~TaskSwitchingCenter()
{
    foreach(auto avr,ThreadBuffer)
    {
        if(avr != NULL)
        {
            delete avr;
            avr = NULL;
        }
    }
    ThreadBuffer.clear();
}

int TaskSwitchingCenter::createProThread( std::string name,CommandDoubleBuffers* parCommandDoubleBuffers /*= NULL*/,EnumProcessType parThreadProcessType /*= EnumProcessType::eParallel*/ )
{
	std::cout<<"createProThread 1"<<std::endl;

    if(parCommandDoubleBuffers)
    {
		QMutexLocker mutexLocker(&m_InsertThreadBufferMutex);
        ThreadBuffer.insert(ThreadBuffer.size(),parCommandDoubleBuffers);
		std::cout<<"createProThread 2"<<std::endl;
        return ThreadBuffer.size() -1;
    }
    else
    {
        CommandDoubleBuffers *pp = new CommandDoubleBuffers(parThreadProcessType);
		printf("TaskSwitchingCenter::createProThread\n");
		QMutexLocker mutexLocker(&m_InsertThreadBufferMutex);
        ThreadBuffer.insert(ThreadBuffer.size(),pp);
		std::cout<<"createProThread 2"<<std::endl;
        return ThreadBuffer.size() -1;
    }
	
}


bool TaskSwitchingCenter::doSerial()
{
	if( m_tempThreadBuffer.size() > 0)
	{
		QMap<ICommandDoubleBuffers*,ICommandDoubleBuffers*> tempThreadBuffer;
		{
			QMutexLocker mutexLocker(&m_InsertThreadBufferMutex);
			tempThreadBuffer = m_tempThreadBuffer;//ThreadBuffer.mid(2,ThreadBuffer.size()-2);
			m_tempThreadBuffer.clear();
		}

		//g_loger.debug("TaskSwitchingCenter::doSerial 1");
		auto beg = tempThreadBuffer.begin();
		auto end = tempThreadBuffer.end();
		for (; beg != end ; beg++)
		{
			(*beg)->doProcess();
		}
		
		//m_semaphoreC.tryAcquire( min(sum,m_semaphoreC.available()) );
		//g_loger.debug("TaskSwitchingCenter::doSerial 2");
	}

	return true;
}
bool TaskSwitchingCenter::doConcurrent()
{
	//int sum = m_semaphoreC.available();
    if( /*sum >0 &&*/ m_tempThreadBuffer.size() > 0)
    {
		QMap<ICommandDoubleBuffers*,ICommandDoubleBuffers*> tempThreadBuffer;
		{
			QMutexLocker mutexLocker(&m_InsertThreadBufferMutex);
			tempThreadBuffer = m_tempThreadBuffer/*ThreadBuffer.mid(2,ThreadBuffer.size()-2)*/;
			m_tempThreadBuffer.clear();
			//sum = m_semaphoreC.available();
		}
        //g_loger.debug("in TaskSwitchingCenter::doConcurrent 1 %d\n",tempThreadBuffer.size());
        watcherDoConcurrent->setFuture(QtConcurrent::map( tempThreadBuffer, FunMapConcurrent) );
        //g_loger.debug("in TaskSwitchingCenter::doConcurrent 2\n");
        watcherDoConcurrent->waitForFinished();
        //g_loger.debug("in TaskSwitchingCenter::doConcurrent 3\n");
		//m_semaphoreC.tryAcquire( min(sum,m_semaphoreC.available()) );
    }
	
    return true;
}
void TaskSwitchingCenter::doProcess()
{
    static int sizeList1 = 0;
    static int sizeList2 = 0;
    while( 1 )
    {
        sizeList1 = m_listCommand[m_bInsertlist].size();
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        if( sizeList2 == 0 )
        {
            if( sizeList1 == 0 )
            {
                QReadLocker locker(&m_readWriteLock);
                m_waitCondition.wait(&m_readWriteLock);
            }
            else
            {
                swapInsertlist();//切换列表
            }
        }
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        foreach (auto var, m_listCommand[!m_bInsertlist])
        {
            //            //printf(" TaskSwitchingCenter::doProcess 1");
            //			if(var->ID>-1 && var->ID < ThreadBuffer.size() )
            //			{
            //				ThreadBuffer[var->ID]->doInsert(var);
            //			}
            //			else
            //			{
            ////printf(" TaskSwitchingCenter::doProcess 2");
            //插入默认列表并且警告
            funDoInsert(var);
            //			}

        }
        m_listCommand[!m_bInsertlist].clear();
    }
}

void TaskSwitchingCenter::funDoInsert( ICommand* command )
{
    ////printf("TaskSwitchingCenter::doInsertDefault\n");
    if( command->m_ProcessType == EnumProcessType(eSerial) )
    {
        ThreadBuffer[0]->doInsert(command);
    }
    else
    {
        ThreadBuffer[1]->doInsert(command);
    }
}

void TaskSwitchingCenter::doInsertDirect( ICommand* command,int ID )
{
    if(ID>-1 && ID < ThreadBuffer.size() )
    {
        ThreadBuffer[ID]->doInsert(command);
		if ( ID > 1)
		{
			//m_semaphoreC.release();
			QMutexLocker mutexLocker(&m_InsertThreadBufferMutex);
			m_tempThreadBuffer[ThreadBuffer[ID]] = ThreadBuffer[ID];
			////g_loger.debug("&---%d--",ThreadBuffer[ID]);
		}
    }
    else
    {
        //插入默认列表并且警告
        TaskSwitchingCenter::funDoInsert(command);
    }
}

void TaskSwitchingCenter::doInsertDefaultParallel( ICommand* command )
{
    ThreadBuffer[0]->doInsert(command);
}

void TaskSwitchingCenter::doInsertDefaultSerial( ICommand* command )
{
    ThreadBuffer[1]->doInsert(command);
}

bool TaskSwitchingCenter::doInsert( ICommand *command )
{
   // ICommandDoubleBuffers::doInsert(command);
	{
		QReadLocker locker(&m_readWriteLock);
		try
		{
			//command->DoPre();
			QMutexLocker mutexLocker(&m_InsertMutex);
			m_listCommand[m_bInsertlist].push_back(command);
		}
		catch (const std::bad_alloc &)
		{
			throw("Error std::bad_alloc in doSerialInsert");
		}
		catch(QString exception)                      //定義異常處理，可以抓取多種類型的異常信息
		{
			throw(exception);
		}
		catch (...)
		{

		}
		////printf("list.push_back\n");
		 m_waitCondition.wakeAll();
		return true;
	}
   
    return false;
}

void TaskSwitchingCenter::run()
{
    doProcess ();
}

void CommandParallel::doProcess()
{
    //g_loger.debug(" in CommandParallel::doProcess");
    static int sizeList1 = 0;
    static int sizeList2 = 0;
    while( 1 )
    {
        sizeList1 = m_listCommand[m_bInsertlist].size();
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        if( sizeList2 == 0 )
        {
            if( sizeList1 == 0 )
            {
                QReadLocker locker(&m_readWriteLock);
                m_waitCondition.wait(&m_readWriteLock);
            }
            else
            {
                swapInsertlist();//切换列表
            }
        }
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        {

            QtConcurrent::blockingMapped(m_listCommand[!m_bInsertlist], FunMapCommand);

        }
        m_listCommand[!m_bInsertlist].clear();
    }
    //g_loger.debug(" out CommandParallel::doProcess");
}

bool CommandParallel::doInsert( ICommand *command )
{
    ICommandDoubleBuffers::doInsert(command);
    m_waitCondition.wakeAll();
    return true;
}

void CommandDoubleBuffers::doParallelProcess()
{
	LOGINOUT
    static int sizeList1 = 0;
    static int sizeList2 = 0;
    while( m_semaphore.available() )
    {
        sizeList1 = m_listCommand[m_bInsertlist].size();
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        if( sizeList2 == 0 )
        {
            if( sizeList1 == 0 )
            {
                return;
            }
            else
            {
                swapInsertlist();//切换列表
            }
        }
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        if (sizeList2)
        {
            watcherDoSerial->setFuture(QtConcurrent::mapped(m_listCommand[!m_bInsertlist], FunMapCommand) );
            watcherDoSerial->waitForFinished();
            m_listCommand[!m_bInsertlist].clear();
			m_semaphore.tryAcquire(__min(sizeList2,m_semaphore.available()));
			//m_semaphore.tryAcquire(sizeList2);

        }
    }
}

void CommandDoubleBuffers::doSerialProcess()
{
	//printf("CommandDoubleBuffers::doSerialProcess 1\n");
	LOGINOUT
    static int sizeList1 = 0;
    static int sizeList2 = 0;
    while(m_semaphore.available()){
        sizeList1 = m_listCommand[m_bInsertlist].size();
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        if( sizeList2 == 0 )
        {
            if( sizeList1 == 0 )
            {
                return;
            }
            else
            {
                swapInsertlist();//切换列表
            }
        }
        sizeList2 = m_listCommand[!m_bInsertlist].size();

        if(sizeList2 )
        {
            auto beg = m_listCommand[!m_bInsertlist].begin();
            auto end = m_listCommand[!m_bInsertlist].end();
			//g_loger.debug("CommandDoubleBuffers::doSerialProcess 2");
            for (; beg != end ; beg++)
            {
                (*beg)->Do();
                delete (*beg);
            }
			//g_loger.debug("CommandDoubleBuffers::doSerialProcess 3");
			m_listCommand[!m_bInsertlist].clear();
			
			m_semaphore.tryAcquire(__min(sizeList2,m_semaphore.available()));
			//g_loger.debug("CommandDoubleBuffers::doSerialProcess 4");
        }

    }
	//printf("CommandDoubleBuffers::doSerialProcess 5\n");
}

CommandDoubleBuffers::CommandDoubleBuffers( EnumProcessType parProcessType )
{
    m_ProcessType = parProcessType;
    watcherDoSerial = new QFutureWatcher<void>();
}

void CommandDoubleBuffers::doProcess()
{
	//printf("in  CommandDoubleBuffers::doProcess\n");
    if(m_ProcessType == EnumProcessType(eSerial) )
    {
        doSerialProcess();
    }
    else
    {
        doParallelProcess();
    }
	//printf("out  CommandDoubleBuffers::doProcess\n");
}

bool CommandDoubleBuffers::doInsert( ICommand* command )
{
    ICommandDoubleBuffers::doInsert(command);
    m_semaphore.release();
    return true;
}

bool ICommandDoubleBuffers::doInsert( ICommand *command )
{
    QReadLocker locker(&m_readWriteLock);
    try
    {
		command->DoPre();
		QMutexLocker mutexLocker(&m_InsertMutex);
        m_listCommand[m_bInsertlist].push_back(command);
    }
    catch (const std::bad_alloc &)
    {
        throw("Error std::bad_alloc in doSerialInsert");
    }
    catch(QString exception)                      //定義異常處理，可以抓取多種類型的異常信息
    {
        throw(exception);
    }
    catch (...)
    {

    }
    ////printf("list.push_back\n");
    return true;
}

ICommandDoubleBuffers::ICommandDoubleBuffers()
{
    m_bInsertlist = false;
    m_listCommand[1].clear();
    m_listCommand[0].clear();
}

void ICommandDoubleBuffers::swapInsertlist()
{
    QWriteLocker locker(&m_readWriteLock);
    m_bInsertlist = !m_bInsertlist;
}

void CommandSerial::doProcess()
{
    static int sizeList1 = 0;
    static int sizeList2 = 0;
    while( 1 )
    {
        sizeList1 = m_listCommand[m_bInsertlist].size();
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        if( sizeList2 == 0 )
        {
            if( sizeList1 == 0 )
            {
                QReadLocker locker(&m_readWriteLock);
                m_waitCondition.wait(&m_readWriteLock);
            }
            else
            {
                swapInsertlist();//切换列表
            }
        }
        sizeList2 = m_listCommand[!m_bInsertlist].size();
        foreach (auto var, m_listCommand[!m_bInsertlist])
        {
            var->Do();

            delete var;
        }
        m_listCommand[!m_bInsertlist].clear();
    }
}

bool CommandSerial::doInsert( ICommand *command )
{
    ICommandDoubleBuffers::doInsert(command);
    m_waitCondition.wakeAll();
    return true;
}
