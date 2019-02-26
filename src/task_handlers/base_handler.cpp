#include "base_handler.h"

bool base_handler::prepare(const std::string& params)
{
    BGN_TRY
    {
        m_duration.start();

        CHK_PRM(this->m_reader.parse(params), "Parse error");

        this->m_id = this->m_reader.get_id();
        this->m_writer.set_id(this->m_id);

        const bool complete = this->prepare_params();
        const bool pending = this->m_result.pending;
        if (!complete && !pending)
        {
            // prepare_params must set an error

            if (!this->m_writer.is_error())
            {
                this->m_writer.reset();
                this->m_writer.set_error(-32602, "Invalid params");
            }
        }

        LOGDEBUG << "Prepared json (" << complete << "," << pending << "):" << this->m_writer.stringify();

        return complete;
    }
    END_TRY_RET(false)
}
