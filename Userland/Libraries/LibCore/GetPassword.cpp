/*
 * Copyright (c) 2020, Peter Elliott <pelliott@ualberta.ca>
 * Copyright (c) 2021, Emanuele Torre <torreemanuele6@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <LibCore/GetPassword.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

namespace Core {

Result<String, OSError> get_password(const StringView& prompt)
{
    if (write(STDOUT_FILENO, prompt.characters_without_null_termination(), prompt.length()) < 0)
        return OSError(errno);

    termios original {};
    if (tcgetattr(STDIN_FILENO, &original) < 0)
        return OSError(errno);

    termios no_echo = original;
    no_echo.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &no_echo) < 0)
        return OSError(errno);

    char* password = nullptr;
    size_t n = 0;

    auto line_length = getline(&password, &n, stdin);
    auto saved_errno = errno;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
    putchar('\n');

    if (line_length < 0)
        return OSError(saved_errno);

    VERIFY(line_length != 0);

    // Remove trailing '\n' read by getline().
    password[line_length - 1] = '\0';

    String s(password);
    free(password);
    return s;
}
}
