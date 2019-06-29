# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

# ------------------------------------------------------------------------------
# Socket readiness
# ------------------------------------------------------------------------------

set(PGFE_SOCKET_READINESS_UNREADY     0)
set(PGFE_SOCKET_READINESS_READ_READY  2)
set(PGFE_SOCKET_READINESS_WRITE_READY 4)
set(PGFE_SOCKET_READINESS_EXCEPTIONS  8)

# ------------------------------------------------------------------------------
# Communication status
# ------------------------------------------------------------------------------

set(PGFE_COMMUNICATION_STATUS_DISCONNECTED            0)
set(PGFE_COMMUNICATION_STATUS_FAILURE               100)
set(PGFE_COMMUNICATION_STATUS_ESTABLISHMENT_WRITING 200)
set(PGFE_COMMUNICATION_STATUS_ESTABLISHMENT_READING 300)
set(PGFE_COMMUNICATION_STATUS_CONNECTED             400)

# ------------------------------------------------------------------------------
# Transaction block status
# ------------------------------------------------------------------------------

set(PGFE_TRANSACTION_BLOCK_STATUS_UNSTARTED     0)
set(PGFE_TRANSACTION_BLOCK_STATUS_UNCOMMITTED 100)
set(PGFE_TRANSACTION_BLOCK_STATUS_FAILED      200)

# ------------------------------------------------------------------------------
# Data format
# ------------------------------------------------------------------------------

set(PGFE_DATA_FORMAT_TEXT   0)
set(PGFE_DATA_FORMAT_BINARY 1)

# ------------------------------------------------------------------------------
# Communication mode
# ------------------------------------------------------------------------------

set(PGFE_COMMUNICATION_MODE_UDS   0)
set(PGFE_COMMUNICATION_MODE_NET 100)

# ------------------------------------------------------------------------------
# Problem_severity
# ------------------------------------------------------------------------------

set(PGFE_PROBLEM_SEVERITY_LOG       0)
set(PGFE_PROBLEM_SEVERITY_INFO    100)
set(PGFE_PROBLEM_SEVERITY_DEBUG   200)
set(PGFE_PROBLEM_SEVERITY_NOTICE  300)
set(PGFE_PROBLEM_SEVERITY_WARNING 400)
set(PGFE_PROBLEM_SEVERITY_ERROR   500)
set(PGFE_PROBLEM_SEVERITY_FATAL   600)
set(PGFE_PROBLEM_SEVERITY_PANIC   700)

# ------------------------------------------------------------------------------
# External_library
# ------------------------------------------------------------------------------

set(PGFE_EXTERNAL_LIBRARY_LIBSSL    2)
set(PGFE_EXTERNAL_LIBRARY_LIBCRYPTO 4)
