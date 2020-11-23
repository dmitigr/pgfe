/* -*- SQL -*-
 * Copyright (C) Dmitry Igrishin
 * For conditions of distribution and use, see files LICENSE.txt
 */

-- This query calculates :n + 1
--
-- $id$plus_one$id$
SELECT :n::int + 1, 'semicolons in qoutes like these: ;;; are ignored';

/*
 * This query concatenates two strings
 *
 * $id$digit$id$
 *
 * $cond$
 * n > 0
 *   AND n < 2
 * $cond$
 */
SELECT n FROM (SELECT generate_series(1,9) n) foo WHERE :cond
