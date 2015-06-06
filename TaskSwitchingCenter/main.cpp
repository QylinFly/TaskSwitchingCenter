#include "taskswitchingcenter.h"

#include <QDebug>
#include <QApplication>
#define QT_NO_CONCURRENT
#define QT_NO_QFUTURE

using namespace PackageQyLinCommon;

class NullCommand:public ICommand
{
public:
	char*mem;
	NullCommand()
	{
		mem = (char*)malloc(1000);
	}
	~NullCommand()
	{
		free(mem);
	}
	virtual void DoPre(){};
    virtual void Do()
    {
    }
    virtual void UnDo()
    {

    }
};

TaskSwitchingCenter m_TaskSwitchingCenter;

int id_1,id_2;

class test11 :public QThread
{
    Q_OBJECT
public:
    test11(){};

    virtual void run ()
    {
        while(1)
        {
			m_TaskSwitchingCenter.doSerial();
			//m_TaskSwitchingCenter.doConcurrent();
			  //this->msleep(10);
        }

    }
};


class test22 :public QThread
{
	Q_OBJECT
public:
	test22(){};

	virtual void run ()
	{
		while(1)
		{
			printf("doInsert\n");
			{
				NullCommand* p;// = new NullCommand();

				NullCommand* pp;// = new NullCommand();

				p = new NullCommand();
				pp = new NullCommand();

				m_TaskSwitchingCenter.doInsertDefaultParallel(p);
				m_TaskSwitchingCenter.doInsertDefaultSerial(pp);
			}
			{
				NullCommand* p;// = new NullCommand();

				NullCommand* pp;// = new NullCommand();

				p = new NullCommand();
				p->ID = id_1;
				pp = new NullCommand();
				pp->ID = id_2;
				printf("--------------1--------------------doInsert\n");
				m_TaskSwitchingCenter.doInsert(p);
				m_TaskSwitchingCenter.doInsert(pp);
				printf("----------------1------------------doInsert\n");
			}
			{
				NullCommand* p;// = new NullCommand();

				NullCommand* pp;// = new NullCommand();

				p = new NullCommand();
				p->ID = id_1;
				pp = new NullCommand();
				pp->ID = id_2;

				m_TaskSwitchingCenter.doInsertDirect(p,p->ID);
				m_TaskSwitchingCenter.doInsertDirect(pp,pp->ID);
			}
			{
				NullCommand* p;// = new NullCommand();

				NullCommand* pp;// = new NullCommand();

				p = new NullCommand();
				pp = new NullCommand();

				m_TaskSwitchingCenter.doInsertDefaultParallel(p);
				m_TaskSwitchingCenter.doInsertDefaultSerial(pp);
			}
			{
				NullCommand* p;// = new NullCommand();

				NullCommand* pp;// = new NullCommand();

				p = new NullCommand();
				p->ID = id_1;
				pp = new NullCommand();
				pp->ID = id_2;
				printf("--------------1--------------------doInsert\n");
				m_TaskSwitchingCenter.doInsert(p);
				m_TaskSwitchingCenter.doInsert(pp);
				printf("----------------1------------------doInsert\n");
			}
			{
				NullCommand* p;// = new NullCommand();

				NullCommand* pp;// = new NullCommand();

				p = new NullCommand();
				p->ID = id_1;
				pp = new NullCommand();
				pp->ID = id_2;

				m_TaskSwitchingCenter.doInsertDirect(p,p->ID);
				m_TaskSwitchingCenter.doInsertDirect(pp,pp->ID);
			}
			//this->msleep(10);
		}

	}
};

#include "main.moc"

int main(int argc, char *argv[])
{
    //QApplication app(argc, argv);
	QCoreApplication app(argc, argv);

	id_1 = m_TaskSwitchingCenter.createProThread("test1");
	id_2 = m_TaskSwitchingCenter.createProThread("test2");

    test11 t;
	test22 t22;
	t22.start();
	t.start();

	test22 t223;
	t223.start();
	test22 t224;
	t224.start();
	test22 t225;
	t225.start();
   // t.startTimer(10);
    
   m_TaskSwitchingCenter.start();
   t.wait();
   m_TaskSwitchingCenter.wait();
   
    return 0;
}


