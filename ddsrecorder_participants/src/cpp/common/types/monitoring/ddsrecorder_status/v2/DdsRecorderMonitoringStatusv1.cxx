// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file DdsRecorderMonitoringStatus.cpp
 * This source file contains the implementation of the described types in the IDL file.
 *
 * This file was generated by the tool fastddsgen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

#include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatus.h>

#if FASTCDR_VERSION_MAJOR == 1

#include <fastcdr/Cdr.h>


#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

namespace helper {
namespace internal {

enum class Size
{
    UInt8,
    UInt16,
    UInt32,
    UInt64,
};

constexpr Size get_size(
        int s)
{
    return (s <= 8 ) ? Size::UInt8:
           (s <= 16) ? Size::UInt16:
           (s <= 32) ? Size::UInt32: Size::UInt64;
}

template<Size s>
struct FindTypeH;

template<>
struct FindTypeH<Size::UInt8>
{
    using type = std::uint8_t;
};

template<>
struct FindTypeH<Size::UInt16>
{
    using type = std::uint16_t;
};

template<>
struct FindTypeH<Size::UInt32>
{
    using type = std::uint32_t;
};

template<>
struct FindTypeH<Size::UInt64>
{
    using type = std::uint64_t;
};
} // namespace internal

template<int S>
struct FindType
{
    using type = typename internal::FindTypeH<internal::get_size(S)>::type;
};
} // namespace helper

#define MonitoringErrorStatus_max_cdr_typesize 6ULL;
#define DdsRecorderMonitoringErrorStatus_max_cdr_typesize 6ULL;
#define DdsRecorderMonitoringStatus_max_cdr_typesize 18ULL;
#define MonitoringStatus_max_cdr_typesize 11ULL;




DdsRecorderMonitoringErrorStatus::DdsRecorderMonitoringErrorStatus()
{
    // boolean m_mcap_file_creation_failure
    m_mcap_file_creation_failure = false;
    // boolean m_disk_full
    m_disk_full = false;

}

DdsRecorderMonitoringErrorStatus::~DdsRecorderMonitoringErrorStatus()
{
}

DdsRecorderMonitoringErrorStatus::DdsRecorderMonitoringErrorStatus(
        const DdsRecorderMonitoringErrorStatus& x)
{
    m_mcap_file_creation_failure = x.m_mcap_file_creation_failure;


    m_disk_full = x.m_disk_full;

}

DdsRecorderMonitoringErrorStatus::DdsRecorderMonitoringErrorStatus(
        DdsRecorderMonitoringErrorStatus&& x) noexcept
{
    m_mcap_file_creation_failure = x.m_mcap_file_creation_failure;


    m_disk_full = x.m_disk_full;

}

DdsRecorderMonitoringErrorStatus& DdsRecorderMonitoringErrorStatus::operator =(
        const DdsRecorderMonitoringErrorStatus& x)
{
    m_mcap_file_creation_failure = x.m_mcap_file_creation_failure;


    m_disk_full = x.m_disk_full;

    return *this;
}

DdsRecorderMonitoringErrorStatus& DdsRecorderMonitoringErrorStatus::operator =(
        DdsRecorderMonitoringErrorStatus&& x) noexcept
{
    m_mcap_file_creation_failure = x.m_mcap_file_creation_failure;


    m_disk_full = x.m_disk_full;

    return *this;
}

bool DdsRecorderMonitoringErrorStatus::operator ==(
        const DdsRecorderMonitoringErrorStatus& x) const
{
    return (m_mcap_file_creation_failure == x.m_mcap_file_creation_failure &&
           m_disk_full == x.m_disk_full);
}

bool DdsRecorderMonitoringErrorStatus::operator !=(
        const DdsRecorderMonitoringErrorStatus& x) const
{
    return !(*this == x);
}

size_t DdsRecorderMonitoringErrorStatus::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return DdsRecorderMonitoringErrorStatus_max_cdr_typesize;
}

size_t DdsRecorderMonitoringErrorStatus::getCdrSerializedSize(
        const DdsRecorderMonitoringErrorStatus& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    return current_alignment - initial_alignment;
}

void DdsRecorderMonitoringErrorStatus::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_mcap_file_creation_failure;

    scdr << m_disk_full;

}

void DdsRecorderMonitoringErrorStatus::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_mcap_file_creation_failure;



    dcdr >> m_disk_full;


}

bool DdsRecorderMonitoringErrorStatus::isKeyDefined()
{
    return false;
}

void DdsRecorderMonitoringErrorStatus::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
}

/*!
 * @brief This function sets a value in member mcap_file_creation_failure
 * @param _mcap_file_creation_failure New value for member mcap_file_creation_failure
 */
void DdsRecorderMonitoringErrorStatus::mcap_file_creation_failure(
        bool _mcap_file_creation_failure)
{
    m_mcap_file_creation_failure = _mcap_file_creation_failure;
}

/*!
 * @brief This function returns the value of member mcap_file_creation_failure
 * @return Value of member mcap_file_creation_failure
 */
bool DdsRecorderMonitoringErrorStatus::mcap_file_creation_failure() const
{
    return m_mcap_file_creation_failure;
}

/*!
 * @brief This function returns a reference to member mcap_file_creation_failure
 * @return Reference to member mcap_file_creation_failure
 */
bool& DdsRecorderMonitoringErrorStatus::mcap_file_creation_failure()
{
    return m_mcap_file_creation_failure;
}

/*!
 * @brief This function sets a value in member disk_full
 * @param _disk_full New value for member disk_full
 */
void DdsRecorderMonitoringErrorStatus::disk_full(
        bool _disk_full)
{
    m_disk_full = _disk_full;
}

/*!
 * @brief This function returns the value of member disk_full
 * @return Value of member disk_full
 */
bool DdsRecorderMonitoringErrorStatus::disk_full() const
{
    return m_disk_full;
}

/*!
 * @brief This function returns a reference to member disk_full
 * @return Reference to member disk_full
 */
bool& DdsRecorderMonitoringErrorStatus::disk_full()
{
    return m_disk_full;
}

DdsRecorderMonitoringStatus::DdsRecorderMonitoringStatus()
    : MonitoringStatus()
{
    // DdsRecorderMonitoringErrorStatus m_ddsrecorder_error_status


}

DdsRecorderMonitoringStatus::~DdsRecorderMonitoringStatus()
{
}

DdsRecorderMonitoringStatus::DdsRecorderMonitoringStatus(
        const DdsRecorderMonitoringStatus& x)
    : MonitoringStatus(x)
{
    m_ddsrecorder_error_status = x.m_ddsrecorder_error_status;

}

DdsRecorderMonitoringStatus::DdsRecorderMonitoringStatus(
        DdsRecorderMonitoringStatus&& x) noexcept
    : MonitoringStatus(std::move(x))
{
    m_ddsrecorder_error_status = std::move(x.m_ddsrecorder_error_status);

}

DdsRecorderMonitoringStatus& DdsRecorderMonitoringStatus::operator =(
        const DdsRecorderMonitoringStatus& x)
{
    MonitoringStatus::operator =(x);

    m_ddsrecorder_error_status = x.m_ddsrecorder_error_status;

    return *this;
}

DdsRecorderMonitoringStatus& DdsRecorderMonitoringStatus::operator =(
        DdsRecorderMonitoringStatus&& x) noexcept
{
    MonitoringStatus::operator =(std::move(x));

    m_ddsrecorder_error_status = std::move(x.m_ddsrecorder_error_status);

    return *this;
}

bool DdsRecorderMonitoringStatus::operator ==(
        const DdsRecorderMonitoringStatus& x) const
{
    if (MonitoringStatus::operator !=(x))
    {
        return false;
    }

    return (m_ddsrecorder_error_status == x.m_ddsrecorder_error_status);
}

bool DdsRecorderMonitoringStatus::operator !=(
        const DdsRecorderMonitoringStatus& x) const
{
    return !(*this == x);
}

size_t DdsRecorderMonitoringStatus::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return DdsRecorderMonitoringStatus_max_cdr_typesize;
}

size_t DdsRecorderMonitoringStatus::getCdrSerializedSize(
        const DdsRecorderMonitoringStatus& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += MonitoringStatus::getCdrSerializedSize(data, current_alignment);

    current_alignment += DdsRecorderMonitoringErrorStatus::getCdrSerializedSize(
        data.ddsrecorder_error_status(), current_alignment);


    return current_alignment - initial_alignment;
}

void DdsRecorderMonitoringStatus::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    MonitoringStatus::serialize(scdr);

    scdr << m_ddsrecorder_error_status;

}

void DdsRecorderMonitoringStatus::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    MonitoringStatus::deserialize(dcdr);

    dcdr >> m_ddsrecorder_error_status;


}

bool DdsRecorderMonitoringStatus::isKeyDefined()
{
    if (MonitoringStatus::isKeyDefined())
    {
        return true;
    }

    return false;
}

void DdsRecorderMonitoringStatus::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
    MonitoringStatus::serializeKey(scdr);
}

/*!
 * @brief This function copies the value in member ddsrecorder_error_status
 * @param _ddsrecorder_error_status New value to be copied in member ddsrecorder_error_status
 */
void DdsRecorderMonitoringStatus::ddsrecorder_error_status(
        const DdsRecorderMonitoringErrorStatus& _ddsrecorder_error_status)
{
    m_ddsrecorder_error_status = _ddsrecorder_error_status;
}

/*!
 * @brief This function moves the value in member ddsrecorder_error_status
 * @param _ddsrecorder_error_status New value to be moved in member ddsrecorder_error_status
 */
void DdsRecorderMonitoringStatus::ddsrecorder_error_status(
        DdsRecorderMonitoringErrorStatus&& _ddsrecorder_error_status)
{
    m_ddsrecorder_error_status = std::move(_ddsrecorder_error_status);
}

/*!
 * @brief This function returns a constant reference to member ddsrecorder_error_status
 * @return Constant reference to member ddsrecorder_error_status
 */
const DdsRecorderMonitoringErrorStatus& DdsRecorderMonitoringStatus::ddsrecorder_error_status() const
{
    return m_ddsrecorder_error_status;
}

/*!
 * @brief This function returns a reference to member ddsrecorder_error_status
 * @return Reference to member ddsrecorder_error_status
 */
DdsRecorderMonitoringErrorStatus& DdsRecorderMonitoringStatus::ddsrecorder_error_status()
{
    return m_ddsrecorder_error_status;
}

#endif // FASTCDR_VERSION_MAJOR == 1
