#include "AsyncTask.h"
#include "core/oxygine.h"
#include <typeinfo>
namespace oxygine
{
#define LOGD(...)  (log::messageln("at: %s (%d) %s",    getName().c_str(), getObjectID(), __VA_ARGS__))
#define LOGD2(...) (log::messageln("at: %s (%d) %s %s", getName().c_str(), getObjectID(), __VA_ARGS__))

    AsyncTask::AsyncTask() : _status(status_not_started), _mainThreadSync(false)
    {
        setName("AsyncTask");
    }

    AsyncTask::~AsyncTask()
    {
        //LOGD("~");
    }

    void AsyncTask::run()
    {
        OX_ASSERT(_status == status_not_started);
        _status = status_inprogress;

        //todo! replace with LOGD
        log::messageln("at: %s (%d) %s %s", getName().c_str(), getObjectID(), "run:", _getRunInfo().c_str());

        bool ok = _prerun();

        if (!ok)
        {
            LOGD("_prerun not ok");
        }


        sync([ = ]()
        {
            if (ok)
                _run();
            else
                _error();
        });
    }

    void AsyncTask::_complete()
    {
        LOGD("_complete");

        _status = status_completed;
        _onFinal(false);
        _finalize(false);
        _onComplete();

        _dispatchComplete();
    }


    void AsyncTask::_dispatchComplete()
    {
        AsyncTaskEvent ev(COMPLETE, this);
        dispatchEvent(&ev);
    }

    void AsyncTask::_error()
    {
        LOGD("_error");


        _status = status_failed;
        _onFinal(true);
        _finalize(true);
        _onError();

        AsyncTaskEvent ev(ERROR, this);
        dispatchEvent(&ev);
    }

    void AsyncTask::onComplete()
    {
        sync([ = ]()
        {
            _complete();
        });
    }

    void AsyncTask::onError()
    {
        sync([ = ]()
        {
            _error();
        });
    }
}