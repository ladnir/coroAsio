#include "Protocol.h"


namespace { // anonymous namespace

    struct ProtocolErrCategory : std::error_category
    {
        const char* name() const noexcept override
        {
            return "ncoro";
        }

        std::string message(int ev) const override
        {
            switch (static_cast<ProtoCodes>(ev))
            {
            case ProtoCodes::success:
                return "Success";
            case ProtoCodes::EndOfRound:
                return "The end of a round has been reached and the protocol was suspended.";
            default:
                return "(unrecognized error)";
            }
        }
    };

    const ProtocolErrCategory theProtoCategory{};

    struct ProtoConditionCategory : std::error_category
    {
        const char* name() const noexcept override
        {
            return "ProtoCatagory";
        }
        std::string message(int ev) const override
        {
            switch (static_cast<ProtoCatagory>(ev))
            {
            case Suspend:
                return "An event occured that caused the protocol to suspend.";
            //case Failure:
            //    return "An error occured that caused the protocol to fail.";
            default:
                return "(unrecognized condition)";
                break;
            }
        }
        bool equivalent(const std::error_code& code,
            int cond) const noexcept override
        {
            if (!code)
                return false;

            switch (static_cast<ProtoCatagory>(cond))
            {
            case ProtoCatagory::Suspend:
                return
                    code == ProtoCodes::EndOfRound ||
                    code == ProtoCodes::PendingRecv ||
                    code == ProtoCodes::PendingSend;
            case ProtoCatagory::Failure:
                return !(code == ProtoCodes::EndOfRound ||
                    code == ProtoCodes::PendingRecv ||
                    code == ProtoCodes::PendingSend);
            default:
                return false;
            }
        }
    };

    const ProtoConditionCategory theProtoConditionCategory{};

} // anonymous namespace


std::error_code make_error_code(const ProtoCodes& e)
{
    return { static_cast<int>(e), theProtoCategory };
}

std::error_condition make_error_condition(const ProtoCatagory& e)
{
    return { static_cast<int>(e), theProtoConditionCategory };
}

