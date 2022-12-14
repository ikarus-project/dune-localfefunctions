# SPDX-FileCopyrightText: 2022 The dune-localfefunction developers mueller@ibb.uni-stuttgart.de
# SPDX-License-Identifier: LGPL-2.1-or-later

/* begin dune-localfefunctions
   put the definitions for config.h specific to
   your project here. Everything above will be
   overwritten
*/

/* begin private */
/* Name of package */
#define PACKAGE "@DUNE_MOD_NAME@"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "@DUNE_MAINTAINER@"

/* Define to the full name of this package. */
#define PACKAGE_NAME "@DUNE_MOD_NAME@"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "@DUNE_MOD_NAME@ @DUNE_MOD_VERSION@"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "@DUNE_MOD_NAME@"

/* Define to the home page for this package. */
#define PACKAGE_URL "@DUNE_MOD_URL@"

/* Define to the version of this package. */
#define PACKAGE_VERSION "@DUNE_MOD_VERSION@"

/* end private */

/* Define to the version of dune-localfefunctions */
#define DUNE_LOCALFEFUNCTIONS_VERSION "@DUNE_LOCALFEFUNCTIONS_VERSION@"

/* Define to the major version of dune-localfefunctions */
#define DUNE_LOCALFEFUNCTIONS_VERSION_MAJOR @DUNE_LOCALFEFUNCTIONS_VERSION_MAJOR@

/* Define to the minor version of dune-localfefunctions */
#define DUNE_LOCALFEFUNCTIONS_VERSION_MINOR @DUNE_LOCALFEFUNCTIONS_VERSION_MINOR@

/* Define to the revision of dune-localfefunctions */
#define DUNE_LOCALFEFUNCTIONS_VERSION_REVISION @DUNE_LOCALFEFUNCTIONS_VERSION_REVISION@

/* Defines a variable to disable code that is only needed for testing */
#define DUNE_LOCALFEFUNCTIONS_ENABLE_TESTING @DUNE_LOCALFEFUNCTIONS_ENABLE_TESTING@

/* Defines a variable to use Eigen for LinearAlgebra */
#cmakedefine DUNE_LOCALFEFUNCTIONS_USE_EIGEN 1

/* end dune-localfefunctions
   Everything below here will be overwritten
*/
