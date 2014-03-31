/* Defines if you have PAM (Pluggable Authentication Modules) */
#cmakedefine HAVE_PAM 1

/* Define if your PAM headers are in pam/ instead of security/ */
#cmakedefine HAVE_PAM_PAM_APPL_H 1

/* Define if your PAM expects a conversation function with const pam_message (Solaris) */
#cmakedefine PAM_MESSAGE_CONST 1

/* The PAM service to be used by kdm */
#cmakedefine KDM_PAM_SERVICE ${KDM_PAM_SERVICE}

/* The PAM service to be used by kscreensaver */
#cmakedefine KSCREENSAVER_PAM_SERVICE ${KSCREENSAVER_PAM_SERVICE}

/* Defines if your system has the getspnam function */
#cmakedefine HAVE_GETSPNAM 1

/* Defines if your system has the crypt function */
#cmakedefine HAVE_CRYPT 1

/* Define to 1 if you have the <crypt.h> header file. */
#cmakedefine HAVE_CRYPT_H 1

/* Define to 1 if you have the `pw_encrypt' function. */
#cmakedefine HAVE_PW_ENCRYPT 1

/* Define to 1 if you have the `getpassphrase' function. */
#cmakedefine HAVE_GETPASSPHRASE 1

/* Define to 1 if you have the `vsyslog' function. */
#cmakedefine HAVE_VSYSLOG 1

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H 1
