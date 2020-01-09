### Mysql备份与恢复

备份：

全量备份和增量备份

全量备份：将整改数据库的数据备份下来。

增量备份：从上一次全量备份后到下一次备份这段时间内，数据库所更新或者增加的数据备份下来，

这时需要打开binlog /etc/mysql/mysql.conf.d/mysqld.cnf，会生成binlog文件，然后用binlog文件进行恢复。

全量备份的脚本：

```sh
#!/bin/bash
# Program
# use mysqldump to Fully backup mysql data per week!
# History
# Path
BakDir=/home/mysql/backup
LogFile=/home/mysql/backup/bak.log
Date=`date +%Y%m%d`
Begin=`date +"%Y年%m月%d日 %H:%M:%S"`
cd $BakDir
DumpFile=$Date.sql
GZDumpFile=$Date.sql.tgz
/usr/local/mysql/bin/mysqldump -uroot -p123456 --quick --events --all-databases --flush-logs --delete-master-logs --single-transaction > $DumpFile
/bin/tar -zvcf $GZDumpFile $DumpFile
/bin/rm $DumpFile
Last=`date +"%Y年%m月%d日 %H:%M:%S"`
echo 开始:$Begin 结束:$Last $GZDumpFile succ >> $LogFile
cd $BakDir/daily
/bin/rm -f *
```

增量备份的脚本：

```sh

#!/bin/bash
BakDir=/home/mysql/backup/daily //增量备份时复制mysql-bin.00000*的目标目录，提前手动创建这个目录
BinDir=/home/mysql/data //mysql的数据目录
LogFile=/home/mysql/backup/bak.log
BinFile=/home/mysql/data/mysql-bin.index //mysql的index文件路径，放在数据目录下的
/usr/local/mysql/bin/mysqladmin -uroot -p123456 flush-logs
#这个是用于产生新的mysql-bin.00000*文件
Counter=`wc -l $BinFile |awk '{print $1}'`
NextNum=0
#这个for循环用于比对$Counter,$NextNum这两个值来确定文件是不是存在或最新的
for file in `cat $BinFile`
do
base=`basename $file`
#basename用于截取mysql-bin.00000*文件名，去掉./mysql-bin.000005前面的./
NextNum=`expr $NextNum + 1`
if [ $NextNum -eq $Counter ]
then
	echo $base skip! >> $LogFile
else
	dest=$BakDir/$base
	if(test -e $dest)
```



然后使用

```sh
[root@ntytcp ~]# crontab -e 
#每个星期日凌晨3:00执行完全备份脚本
0 3 * * 0 /bin/bash -x /root/Mysql-FullyBak.sh >/dev/null 2>&1 
#周一到周六凌晨3:00做增量备份
0 3 * * 1-6 /bin/bash -x /root/Mysql-DailyBak.sh >/dev/null 2>&1
```






