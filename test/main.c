/*
 * This file is part of nmealib.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

extern int contextSuiteSetup(void);
extern int gpggaSuiteSetup(void);
extern int gpgsaSuiteSetup(void);
extern int gpvtgSuiteSetup(void);

int main(void) {
  if (CUE_SUCCESS != CU_initialize_registry()) {
    goto out;
  }

  if ( //
      (contextSuiteSetup() != CUE_SUCCESS) //
      || (gpggaSuiteSetup() != CUE_SUCCESS) //
      || (gpgsaSuiteSetup() != CUE_SUCCESS) //
      || (gpvtgSuiteSetup() != CUE_SUCCESS) //
      ) {
    goto cleanup;
  }

  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);

  CU_basic_run_tests();

cleanup:
  CU_cleanup_registry();

out:
  return CU_get_error();
}
