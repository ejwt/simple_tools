#!/bin/bash

# make directory list; only supports one-level of directory
ls -d */ | sed -e "s;\/;;" > dir_list.tmp

for DIR in `cat dir_list.tmp`
do
    cd $DIR
    
    rm -f *.m3u8

    # make file list
    ls > file_list.tmp

    for FILE  in `cat file_list.tmp | grep mid`
    do
        mv $FILE `echo $FILE | cut -c 45- | sed -e "s;.ts;;"`
    done

    rm -f file_list.tmp

    mv  mid0 mid000
    mv  mid1 mid001
    mv  mid2 mid002
    mv  mid3 mid003
    mv  mid4 mid004
    mv  mid5 mid005
    mv  mid6 mid006
    mv  mid7 mid007
    mv  mid8 mid008
    mv  mid9 mid009
    mv mid10 mid010
    mv mid11 mid011
    mv mid12 mid012
    mv mid13 mid013
    mv mid14 mid014
    mv mid15 mid015
    mv mid16 mid016
    mv mid17 mid017
    mv mid18 mid018
    mv mid19 mid019
    mv mid20 mid020
    mv mid21 mid021
    mv mid22 mid022
    mv mid23 mid023
    mv mid24 mid024
    mv mid25 mid025
    mv mid26 mid026
    mv mid27 mid027
    mv mid28 mid028
    mv mid29 mid029
    mv mid30 mid030
    mv mid31 mid031
    mv mid32 mid032
    mv mid33 mid033
    mv mid34 mid034
    mv mid35 mid035
    mv mid36 mid036
    mv mid37 mid037
    mv mid38 mid038
    mv mid39 mid039
    mv mid40 mid040
    mv mid41 mid041
    mv mid42 mid042
    mv mid43 mid043
    mv mid44 mid044
    mv mid45 mid045
    mv mid46 mid046
    mv mid47 mid047
    mv mid48 mid048
    mv mid49 mid049
    mv mid50 mid050
    mv mid51 mid051
    mv mid52 mid052
    mv mid53 mid053
    mv mid54 mid054
    mv mid55 mid055
    mv mid56 mid056
    mv mid57 mid057
    mv mid58 mid058
    mv mid59 mid059
    mv mid60 mid060
    mv mid61 mid061
    mv mid62 mid062
    mv mid63 mid063
    mv mid64 mid064
    mv mid65 mid065
    mv mid66 mid066
    mv mid67 mid067
    mv mid68 mid068
    mv mid69 mid069
    mv mid70 mid070
    mv mid71 mid071
    mv mid72 mid072
    mv mid73 mid073
    mv mid74 mid074
    mv mid75 mid075
    mv mid76 mid076
    mv mid77 mid077
    mv mid78 mid078
    mv mid79 mid079
    mv mid80 mid080
    mv mid81 mid081
    mv mid82 mid082
    mv mid83 mid083
    mv mid84 mid084
    mv mid85 mid085
    mv mid86 mid086
    mv mid87 mid087
    mv mid88 mid088
    mv mid89 mid089
    mv mid90 mid090
    mv mid91 mid091
    mv mid92 mid092
    mv mid93 mid093
    mv mid94 mid094
    mv mid95 mid095
    mv mid96 mid096
    mv mid97 mid097
    mv mid98 mid098
    mv mid99 mid099

    cat mid* > ../${DIR}_.ts

    cd ..
done

rm -f dir_list.tmp
