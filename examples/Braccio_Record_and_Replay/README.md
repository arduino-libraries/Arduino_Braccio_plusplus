`Braccio_Record_and_Replay`
===========================
For debug purposes all recorded angles can be received via serial command. The approach to do this is:
* Start `minicom`.
```bash
minicom -D /dev/ttyACM0  -C angle_log.csv
```
* Press `Record` and record some angles.
* Retrieve the recorded angles by sending `r` (Press `r`). You'll see the output on your console which will automagically be stored in the CSV file.
* Close `minicom` with `Ctrl + A`, `Ctrl + Q`.
