#!/bin/sh

# Send an email to all the GNU PDF hackers

RECFILE=$HOME/org/gnupdf-hackers.org

for addr in `recsel -t Hacker -p /Email[0] $RECFILE`
do
   mail $addr ...
done 

# Send an email to the secondary email of hackers

RECFILE=$HOME/org/gnupdf-hackers.org

if !recfix $RECFILE
then
   echo "Invalid data."
   exit
fi

NUMRECS=`recsel -t Hacker -e "#Email > 1" -c $RECFILE`
NREC=0
while test $NREC -lt $NUMRECS
do
   REC=`recsel -t Hacker -n $NREC $RECFILE`
   for addr in `echo $REC | recsel -p /Email[1..]`
   do
     NAME=`echo $REC | recsel -p %Name`
     DESCR=`echo $REC | recsel -p %Descr`
     mail $NAME $addr ...
   done
done

# Print the description of the packages maintained by Foo
#
# GNU.rec
#
#   %rec: Hacker
#   %mandatory: Email
#   
#   Name: Foo
#   Email: foo@bar.org
#   Package:Name: pkg1
#   Package:Name: pkg2
#
#   %rec: Package
#
#   Name: pkg1
#   Description: foo
#   + bar
#   URL: http://www.pkg1.com
#
#   Name: pkg2
#   Description: foo
#   + jorl

RECFILE=$HOME/org/GNU.rec

if !recfix $RECFILE
then
   echo "Invalid data."
   exit
fi

HACKERS=`recsel -t Hacker -e "#Package:Name" $RECFILE`
NHACKERS=`echo $HACKERS | recsel -c`

NHACKER=0
while test $NHACKER -gt $NHACKERS
do
   HACKER=`echo $HACKERS | recsel -n $NHACKER`
   HACKER_NAME=`echo $HACKER | recsel -p %Name`
   HACKER_EMAIL=`echo $HACKER | recsel -p %Email`

   for pname in `echo $HACKER | recsel -p %Package:Name`
   do
      PDESCRIPTION=`recsel -t Package -e "Name = $pname" -p %Description $RECFILE`
      ... use PDESCRIPTION ...
   done
done


# Print the description of the packages maintained by Foo (with join)
#
# GNU.rec
#
#   %rec: Hacker
#   %mandatory: Email
#   
#   Name: Foo
#   Email: foo@bar.org
#   Package:Name: pkg1
#   Package:Name: pkg2
#
#   %rec: Package
#   %key: Name
#
#   Name: pkg1
#   Description: foo
#   + bar
#   URL: http://www.pkg1.com
#
#   Name: pkg2
#   Description: foo
#   + jorl

RECFILE=$HOME/org/GNU.rec

if !recfix $RECFILE
then
   echo "Invalid data."
   exit
fi

RECS=`recsel -t Hacker -e "#Name:Package" -p Name,Hacker:Email,Package:Name,Package:Description $RECFILE`
# RECS:
#
#   Name: Foo
#   Hacker:Email: foo@bar.org
#   Package:Name: pkg1
#   Package:Description: foo
#   + bar
#
#   Name: Foo
#   Hacker:Email: foo@bar.org
#   Package:Name: pkg2
#   Package:Description: foo
#   + jorl

NRECS=`echo $RECS | recsel -c`

NREC=0
while test $NREC -gt $NRECS
do
   REC=`echo $RECS | recsel -n $NREC`
   HACKER_NAME=`echo $REC | recsel -p %Name`
   HACKER_EMAIL=`echo $REC | recsel -p %Hacker:Email`
   PACKAGE_DESCRIPTOR=`echo $REC | recsel -p %Package:Description`
done

# Monitoring the free memory in the system

OUTFILE=/var/log/memlog.rec

while true
do
   MEM=`free | grep Mem: | awk  '{ print $2; }'`
   DATE=`date`

   recins -n Memory -v $MEM -n Date -v $DATE $OUTFILE

   # Wait for 20 minutes
   sleep `expr 20 * 60`
done

# Add new hacker to GNU.rec

recins -t Hacker -n Name -v 'Juan Valdes' -n Email -v juan@valdes.org GNU.rec

# Add a new email for a hacker in GNU.rec

recins -t Hacker -e "Name = 'Juan Valdes'" -n Email -v another@email.org GNU.rec

# Delete all the Emails from juan valdes

recdel -t Hacker -e "Name = 'Juan Valdes'" -n Email

# Delete the first Email of juan valdes

recdel -t Hacker -e "Name = 'Juan Valdes'" -n Email[0]

# Delete Juan Valdes

recdel -t Hacker -e "Name = 'Juan Valdes'"

# Change an email of Juan Valdes from juan@valdes.org to juan@mail.org

recset -t Hacker -e "Name = 'Juan Valdes'" -n Email juan@valdes.org juan@mail.org


# End of foo.sh
