#include <stdlib.h>
#include <cm.h>

CMthreadJob_p CMthreadJobCreate (size_t taskNum, CMthreadUserFunc userFunc, void *userData) {
	size_t taskId;
	CMthreadJob_p job;

	if ((job = (CMthreadJob_p) malloc (sizeof (CMthreadJob_t))) == (CMthreadJob_p) NULL) {
		CMmsgPrint (CMmsgSysError, "Memory allocation error in %s:%d\n",__FILE__,__LINE__);
		return ((CMthreadJob_p) NULL);
	}
	if ((job->Tasks = (CMthreadTask_p) calloc (taskNum,sizeof (CMthreadTask_t))) == (CMthreadTask_p) NULL) {
		CMmsgPrint (CMmsgSysError, "Memory allocation error in %s:%d\n",__FILE__,__LINE__);
		free (job);
		return ((CMthreadJob_p) NULL);
	}
	job->TaskNum = taskNum;
	for (taskId = 0;taskId < job->TaskNum; ++taskId) {
		job->Tasks [taskId].Id         = taskId;
		job->Tasks [taskId].Completed  = false;
		job->Tasks [taskId].Locked     = false;
		job->Tasks [taskId].Dependence = taskId;
	}
	job->LastId   = 0;
	job->UserFunc = userFunc;
	job->UserData = (void *) userData;
	return (job);
}

CMreturn CMthreadJobTaskDependece (CMthreadJob_p job, size_t taskId, size_t dependence) {
	if (taskId > job->TaskNum) {
		CMmsgPrint (CMmsgAppError,"Invalid task in %s%d\n",__FILE__,__LINE__);
		return (CMfailed);
	}
	if (dependence > job->TaskNum) {
		CMmsgPrint (CMmsgAppError,"Invalid task dependence in %s%d\n",__FILE__,__LINE__);
		return (CMfailed);
	}
	if (job->Tasks [taskId].Dependence < dependence) job->Tasks [taskId].Dependence = dependence;
	return (CMsucceeded);
}

void CMthreadJobDestroy (CMthreadJob_p job) {
	free (job->Tasks);
	free (job);
}
static void *_CMthreadWork (void *dataPtr) {
	CMthreadData_p data = (CMthreadData_p) dataPtr;
	size_t taskId;
	bool cont;
	CMthreadTeam_p team = (CMthreadTeam_p) data->TeamPtr;
	CMthreadJob_p  job;

	pthread_mutex_lock   (&(team->Mutex));
	pthread_cond_signal  (&(team->SlaveSignal));
	pthread_mutex_unlock (&(team->Mutex));

	 while (true) {
		pthread_mutex_lock   (&(team->Mutex));
		printf ("Thread#%d: Waiting for Begin condition\n",(int) data->Id);
		pthread_cond_wait    (&(team->MasterSignal), &(team->Mutex));
		job = team->Job;
		pthread_mutex_unlock (&(team->Mutex));
		if (job == (CMthreadJob_p) NULL) {
			printf ("Thread#%d: Quitting\n",(int) data->Id);
			break;
		}
		cont = true;
		do {
			pthread_mutex_lock (&(team->Mutex));
			taskId = job->LastId;
			job->LastId++;
			if (job->LastId == job->TaskNum) {
				printf ("Thread#%d: Sending completation signal to master\n",(int) data->Id);
				pthread_cond_signal  (&team->SlaveSignal);
			}
			pthread_mutex_unlock (&(team->Mutex));
			if (taskId  < job->TaskNum) {
				job->UserFunc (job->UserData, taskId);
				data->CompletedTasks++;
			}
			else {
				cont = false;
				printf ("Thread#%d: Job Finished\n",(int) data->Id);
			}
		} while (cont);
	}
	pthread_exit((void *) 0);
}

CMthreadTeam_p CMthreadTeamCreate (size_t threadNum) {
	int ret;
	size_t threadId;
	pthread_attr_t thread_attr;
	CMthreadTeam_p team;

	if ((team = (CMthreadTeam_p) malloc (sizeof (CMthreadTeam_t))) == (CMthreadTeam_p) NULL) {
		CMmsgPrint (CMmsgSysError,"Memory Allocation error in %s:%d\n",__FILE__,__LINE__);
		return ((CMthreadTeam_p) NULL);
	}
	if ((team->Threads = (CMthreadData_p) calloc (threadNum, sizeof(CMthreadData_t))) == (CMthreadData_p) NULL) {
		CMmsgPrint (CMmsgSysError,"Memory Allocation error in %s:%d\n",__FILE__,__LINE__);
		free (team);
		return ((CMthreadTeam_p) NULL);
	}
	team->ThreadNum = threadNum;
	team->Job       = (CMthreadJob_p) NULL;

	pthread_mutex_init (&(team->Mutex),        NULL);
	pthread_cond_init  (&(team->MasterSignal), NULL);
	pthread_cond_init  (&(team->SlaveSignal),  NULL);

	pthread_attr_init (&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

	pthread_mutex_lock (&(team->Mutex));
	for (threadId = 0; threadId < team->ThreadNum; ++threadId) {
		team->Threads [threadId].Id             = threadId;
		team->Threads [threadId].CompletedTasks = 0;
		team->Threads [threadId].TeamPtr        = (void *) team;
		if ((ret = pthread_create (&(team->Threads [threadId].Thread), &thread_attr,_CMthreadWork,(void *) (team->Threads + threadId))) != 0) {
			CMmsgPrint (CMmsgAppError,"Thread creation returned with error [%d] in %s:%d\n",ret,__FILE__,__LINE__);
			free (team->Threads);
			free (team);
			return ((CMthreadTeam_p) NULL);
		}
		pthread_cond_wait      (&(team->SlaveSignal), &(team->Mutex));
	}
	pthread_mutex_unlock (&(team->Mutex));
	pthread_attr_destroy(&thread_attr);
	return (team);
}

void CMthreadTeamJobExecute (CMthreadTeam_p team, CMthreadJob_p job) {
	pthread_mutex_lock     (&(team->Mutex));
	printf ("Master: broadcasting a begin signal\n");
	team->Job   = job;
	job->LastId = 0;

	pthread_cond_broadcast (&(team->MasterSignal));
	pthread_cond_wait      (&(team->SlaveSignal), &(team->Mutex));
	printf ("Master: Received an end signal\n");
	team->Job  = (CMthreadJob_p) NULL;
	pthread_mutex_unlock   (&(team->Mutex));
}

void CMthreadTeamDestroy (CMthreadTeam_p team) {
	size_t threadId;
	size_t completedTasks = 0;
	void  *status;

	pthread_mutex_lock (&(team->Mutex));
	team->Job = (CMthreadJob_p) NULL;
	pthread_cond_broadcast (&(team->MasterSignal));
	pthread_mutex_unlock (&(team->Mutex));
	for (threadId = 0;threadId < team->ThreadNum;++threadId) {
		pthread_join(team->Threads [threadId].Thread, &status);
		completedTasks += team->Threads [threadId].CompletedTasks;
	}
	for (threadId = 0;threadId < team->ThreadNum;++threadId) {
		printf ("Threads#%d completed %d tasks (%6.2f of the total)\n",
				(int)   team->Threads [threadId].Id,
				(int)   team->Threads [threadId].CompletedTasks,
				(float) team->Threads [threadId].CompletedTasks * 100.0 / (float) completedTasks);
	}
	pthread_mutex_destroy (&(team->Mutex));
	pthread_cond_destroy  (&(team->MasterSignal));
	pthread_cond_destroy  (&(team->SlaveSignal));
	pthread_exit(NULL);
	free (team->Threads);
	free (team);
}