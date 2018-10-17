#include "task_handlers.h"

template <class T>
bool base_handler<T>::prepare(const std::string& params)
{
    if (!this->m_reader.parse(params))
    {
        STREAM_LOG_DBG("Incorrect json " << params)
        this->m_writer.set_error(-32701, "Parse error");
        return false;
    }

    this->m_id = this->m_reader.get_id();

    bool success = this->prepare_params();
    if (!success)
    {
        // prepare_params must set an error

        if (!this->m_writer.is_error())
        {
            this->m_writer.reset();
            this->m_writer.set_error(-32602, "Invalid params");
        }
    }

    this->m_writer.set_id(this->m_id);

    STREAM_LOG_DBG("Prepared json (" << success << "):" << std::endl << this->m_writer.stringify())

    return success;
}
