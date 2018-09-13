# Network Time Protocol Clock

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/73f8886f119a4410903a0240658d6c6b)](https://app.codacy.com/app/duncan.ian.t/NTP-Clock?utm_source=github.com&utm_medium=referral&utm_content=IanDuncanT/NTP-Clock&utm_campaign=Badge_Grade_Settings)
***
This network time protocol clock querries a time server and returns the tkme to a lcd screen over 4 bits in parallel. The default time refresh interval is 30 seconds and the current code is set to Central Standard Time. The time offset and refresh interval can be changed in the global ints near the top using UTC offset.