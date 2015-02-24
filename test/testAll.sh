#!/bin/bash

PRG=""

if [ $# -eq 0 ] 
then
    exit 1;
fi

if [ $# -eq 1 ]
then
    PRG=$1
fi

WDIR=`dirname $PRG`

RVAL=0
echo "Testing neutrality test statistics"
if ./testTaj.sh $WDIR ;then
    echo "Problem with neutrality test statistics exit code: $?"
    cat ./testTaj.sh.log
    RVAL=1
fi
echo "Testing SFS"
if ./testSFS.sh $WDIR ;then
    echo "Problem with SFS exit code: $?"
    cat ./testSFS.sh.log
    RVAL=1
fi
exit ${RVAL}

if false; then
    echo ./testBam.sh $PRG
    ./testBam.sh $PRG
    
    echo testAsso6.sh $PRG
    ./testAsso6.sh $PRG
    
    #echo ./testErr.sh $PRG
    #./testErr.sh $PRG
    
    echo ./testGL6.sh $PRG
    ./testGL6.sh $PRG
    
    echo ./testMisc9.sh $PRG
    ./testMisc9.sh $PRG
    
    echo ./testAbba.sh $PRG
    ./testAbba.sh $PRG
        
    echo ./testFasta.sh
    ./testFasta.sh $PRG
    
    echo ./testBaq.sh $PRG
    ./testBaq3.sh $PRG
    
    echo "Netaccess is now deprecated"
    #echo ./testNetAccess.sh $PRG
    #./testNetAccess.sh $PRG
fi
