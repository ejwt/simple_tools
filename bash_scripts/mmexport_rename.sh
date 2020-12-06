#!/bin/bash

# ========================== mmexport*.jpg/jpeg ==========================
# make file list
ls *.jpg > filenames.tmp
ls *.jpeg >> filenames.tmp

for FILE in `cat filenames.tmp | grep mmexport`
do
    unix_time=`echo ${FILE} | cut -c 9-21`
    echo 'Unix time: '${unix_time}

    # convert UTC to UTC+8 (CST: China Standard Time)
    # 28800000 = 3600*8*1000
    unix_time=$(( ${unix_time} + 28800000 ))

    # There're 31536000000 miliseconds in a common year.
    # 31536000000 = 1000*86400*365
    year=$(( $((${unix_time} / 31536000000)) + 1970 ))

    # date: how many days since the first day in this year, *excluding* the current day!!
    # i.e. Jan. 1 --> 0
    #      Jan. 2 --> 1
    #      Feb. 1 --> 31
    # There're 86,400,000 miliseconds in a day.
    # $((${year} - 1972)) / 4: how many leap days since 1970, *excluding* the current year!!
    date=$(( ${unix_time} % 31536000000 / 86400000 - $((${year} - 1972)) / 4 ))
    if [ ${date} -lt 0 ]; then
        date=$(( ${date} + 365 ))
        year=$(( ${year} - 1 ))
    fi

    hour=$(( ${unix_time} % 86400000 / 3600000 ))
    minute=$(( ${unix_time} % 3600000 / 60000 ))
    second=$(( ${unix_time} % 60000 / 1000 ))
    ms=$(( ${unix_time} % 1000 ))

    # check if it's in a leap year
    if [ $((${year} % 4)) -eq 0 ]; then
        #echo 'leap year!'
        if [ ${date} -ge 335 ]; then
            month=12
            days_before_this_month=335
        elif [ ${date} -ge 305 ]; then
            month=11
            days_before_this_month=305
        elif [ ${date} -ge 274 ]; then
            month=10
            days_before_this_month=274
        elif [ ${date} -ge 244 ]; then
            month=9
            days_before_this_month=244
        elif [ ${date} -ge 213 ]; then
            month=8
            days_before_this_month=213
        elif [ ${date} -ge 182 ]; then
            month=7
            days_before_this_month=182
        elif [ ${date} -ge 152 ]; then
            month=6
            days_before_this_month=152
        elif [ ${date} -ge 121 ]; then
            month=5
            days_before_this_month=121
        elif [ ${date} -ge 91 ]; then
            month=4
            days_before_this_month=91
        elif [ ${date} -ge 60 ]; then
            month=3
            days_before_this_month=60
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi

    else
        #echo 'common year'
        if [ ${date} -ge 334 ]; then
            month=12
            days_before_this_month=334
        elif [ ${date} -ge 304 ]; then
            month=11
            days_before_this_month=304
        elif [ ${date} -ge 273 ]; then
            month=10
            days_before_this_month=273
        elif [ ${date} -ge 243 ]; then
            month=9
            days_before_this_month=243
        elif [ ${date} -ge 212 ]; then
            month=8
            days_before_this_month=212
        elif [ ${date} -ge 181 ]; then
            month=7
            days_before_this_month=181
        elif [ ${date} -ge 151 ]; then
            month=6
            days_before_this_month=151
        elif [ ${date} -ge 120 ]; then
            month=5
            days_before_this_month=120
        elif [ ${date} -ge 90 ]; then
            month=4
            days_before_this_month=90
        elif [ ${date} -ge 59 ]; then
            month=3
            days_before_this_month=59
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi
    fi

    day=$(( ${date} - ${days_before_this_month} + 1 ))

    # add leading zeros
    if [ ${month} -lt 10 ]; then
        str_month=`echo 0${month}`
    else
        str_month=`echo ${month}`
    fi

    if [ ${day} -lt 10 ]; then
        str_day=`echo 0${day}`
    else
        str_day=`echo ${day}`
    fi

    if [ ${hour} -lt 10 ]; then
        str_hour=`echo 0${hour}`
    else
        str_hour=`echo ${hour}`
    fi

    if [ ${minute} -lt 10 ]; then
        str_minute=`echo 0${minute}`
    else
        str_minute=`echo ${minute}`
    fi

    if [ ${second} -lt 10 ]; then
        str_second=`echo 0${second}`
    else
        str_second=`echo ${second}`
    fi

    if [ ${ms} -lt 10 ]; then
        str_ms=`echo 00${ms}`
    elif [ ${ms} -lt 100 ]; then
        str_ms=`echo 0${ms}`
    else
        str_ms=`echo ${ms}`
    fi

    mv ${FILE} ${year}-${str_month}-${str_day}_${str_hour}-${str_minute}-${str_second}.${str_ms}.jpg
done


# ========================== mmexport*.png ==========================
ls *.png > filenames.tmp

for FILE in `cat filenames.tmp | grep mmexport`
do
    unix_time=`echo ${FILE} | cut -c 9-21`
    echo 'Unix time: '${unix_time}

    # convert UTC to UTC+8 (CST: China Standard Time)
    # 28800000 = 3600*8*1000
    unix_time=$(( ${unix_time} + 28800000 ))

    # There're 31536000000 miliseconds in a common year.
    # 31536000000 = 1000*86400*365
    year=$(( $((${unix_time} / 31536000000)) + 1970 ))

    # date: how many days since the first day in this year, *excluding* the current day!!
    # i.e. Jan. 1 --> 0
    #      Jan. 2 --> 1
    #      Feb. 1 --> 31
    # There're 86,400,000 miliseconds in a day.
    # $((${year} - 1972)) / 4: how many leap days since 1970, *excluding* the current year!!
    date=$(( ${unix_time} % 31536000000 / 86400000 - $((${year} - 1972)) / 4 ))
    if [ ${date} -lt 0 ]; then
        date=$(( ${date} + 365 ))
        year=$(( ${year} - 1 ))
    fi

    hour=$(( ${unix_time} % 86400000 / 3600000 ))
    minute=$(( ${unix_time} % 3600000 / 60000 ))
    second=$(( ${unix_time} % 60000 / 1000 ))
    ms=$(( ${unix_time} % 1000 ))

    # check if it's in a leap year
    if [ $((${year} % 4)) -eq 0 ]; then
        #echo 'leap year!'
        if [ ${date} -ge 335 ]; then
            month=12
            days_before_this_month=335
        elif [ ${date} -ge 305 ]; then
            month=11
            days_before_this_month=305
        elif [ ${date} -ge 274 ]; then
            month=10
            days_before_this_month=274
        elif [ ${date} -ge 244 ]; then
            month=9
            days_before_this_month=244
        elif [ ${date} -ge 213 ]; then
            month=8
            days_before_this_month=213
        elif [ ${date} -ge 182 ]; then
            month=7
            days_before_this_month=182
        elif [ ${date} -ge 152 ]; then
            month=6
            days_before_this_month=152
        elif [ ${date} -ge 121 ]; then
            month=5
            days_before_this_month=121
        elif [ ${date} -ge 91 ]; then
            month=4
            days_before_this_month=91
        elif [ ${date} -ge 60 ]; then
            month=3
            days_before_this_month=60
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi

    else
        #echo 'common year'
        if [ ${date} -ge 334 ]; then
            month=12
            days_before_this_month=334
        elif [ ${date} -ge 304 ]; then
            month=11
            days_before_this_month=304
        elif [ ${date} -ge 273 ]; then
            month=10
            days_before_this_month=273
        elif [ ${date} -ge 243 ]; then
            month=9
            days_before_this_month=243
        elif [ ${date} -ge 212 ]; then
            month=8
            days_before_this_month=212
        elif [ ${date} -ge 181 ]; then
            month=7
            days_before_this_month=181
        elif [ ${date} -ge 151 ]; then
            month=6
            days_before_this_month=151
        elif [ ${date} -ge 120 ]; then
            month=5
            days_before_this_month=120
        elif [ ${date} -ge 90 ]; then
            month=4
            days_before_this_month=90
        elif [ ${date} -ge 59 ]; then
            month=3
            days_before_this_month=59
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi
    fi

    day=$(( ${date} - ${days_before_this_month} + 1 ))

    # add leading zeros
    if [ ${month} -lt 10 ]; then
        str_month=`echo 0${month}`
    else
        str_month=`echo ${month}`
    fi

    if [ ${day} -lt 10 ]; then
        str_day=`echo 0${day}`
    else
        str_day=`echo ${day}`
    fi

    if [ ${hour} -lt 10 ]; then
        str_hour=`echo 0${hour}`
    else
        str_hour=`echo ${hour}`
    fi

    if [ ${minute} -lt 10 ]; then
        str_minute=`echo 0${minute}`
    else
        str_minute=`echo ${minute}`
    fi

    if [ ${second} -lt 10 ]; then
        str_second=`echo 0${second}`
    else
        str_second=`echo ${second}`
    fi

    if [ ${ms} -lt 10 ]; then
        str_ms=`echo 00${ms}`
    elif [ ${ms} -lt 100 ]; then
        str_ms=`echo 0${ms}`
    else
        str_ms=`echo ${ms}`
    fi

    mv ${FILE} ${year}-${str_month}-${str_day}_${str_hour}-${str_minute}-${str_second}.${str_ms}.png
done


# ========================== mmexport*.gif ==========================
ls *.gif > filenames.tmp

for FILE in `cat filenames.tmp | grep mmexport`
do
    unix_time=`echo ${FILE} | cut -c 9-21`
    echo 'Unix time: '${unix_time}

    # convert UTC to UTC+8 (CST: China Standard Time)
    # 28800000 = 3600*8*1000
    unix_time=$(( ${unix_time} + 28800000 ))

    # There're 31536000000 miliseconds in a common year.
    # 31536000000 = 1000*86400*365
    year=$(( $((${unix_time} / 31536000000)) + 1970 ))

    # date: how many days since the first day in this year, *excluding* the current day!!
    # i.e. Jan. 1 --> 0
    #      Jan. 2 --> 1
    #      Feb. 1 --> 31
    # There're 86,400,000 miliseconds in a day.
    # $((${year} - 1972)) / 4: how many leap days since 1970, *excluding* the current year!!
    date=$(( ${unix_time} % 31536000000 / 86400000 - $((${year} - 1972)) / 4 ))
    if [ ${date} -lt 0 ]; then
        date=$(( ${date} + 365 ))
        year=$(( ${year} - 1 ))
    fi

    hour=$(( ${unix_time} % 86400000 / 3600000 ))
    minute=$(( ${unix_time} % 3600000 / 60000 ))
    second=$(( ${unix_time} % 60000 / 1000 ))
    ms=$(( ${unix_time} % 1000 ))

    # check if it's in a leap year
    if [ $((${year} % 4)) -eq 0 ]; then
        #echo 'leap year!'
        if [ ${date} -ge 335 ]; then
            month=12
            days_before_this_month=335
        elif [ ${date} -ge 305 ]; then
            month=11
            days_before_this_month=305
        elif [ ${date} -ge 274 ]; then
            month=10
            days_before_this_month=274
        elif [ ${date} -ge 244 ]; then
            month=9
            days_before_this_month=244
        elif [ ${date} -ge 213 ]; then
            month=8
            days_before_this_month=213
        elif [ ${date} -ge 182 ]; then
            month=7
            days_before_this_month=182
        elif [ ${date} -ge 152 ]; then
            month=6
            days_before_this_month=152
        elif [ ${date} -ge 121 ]; then
            month=5
            days_before_this_month=121
        elif [ ${date} -ge 91 ]; then
            month=4
            days_before_this_month=91
        elif [ ${date} -ge 60 ]; then
            month=3
            days_before_this_month=60
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi

    else
        #echo 'common year'
        if [ ${date} -ge 334 ]; then
            month=12
            days_before_this_month=334
        elif [ ${date} -ge 304 ]; then
            month=11
            days_before_this_month=304
        elif [ ${date} -ge 273 ]; then
            month=10
            days_before_this_month=273
        elif [ ${date} -ge 243 ]; then
            month=9
            days_before_this_month=243
        elif [ ${date} -ge 212 ]; then
            month=8
            days_before_this_month=212
        elif [ ${date} -ge 181 ]; then
            month=7
            days_before_this_month=181
        elif [ ${date} -ge 151 ]; then
            month=6
            days_before_this_month=151
        elif [ ${date} -ge 120 ]; then
            month=5
            days_before_this_month=120
        elif [ ${date} -ge 90 ]; then
            month=4
            days_before_this_month=90
        elif [ ${date} -ge 59 ]; then
            month=3
            days_before_this_month=59
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi
    fi

    day=$(( ${date} - ${days_before_this_month} + 1 ))

    # add leading zeros
    if [ ${month} -lt 10 ]; then
        str_month=`echo 0${month}`
    else
        str_month=`echo ${month}`
    fi

    if [ ${day} -lt 10 ]; then
        str_day=`echo 0${day}`
    else
        str_day=`echo ${day}`
    fi

    if [ ${hour} -lt 10 ]; then
        str_hour=`echo 0${hour}`
    else
        str_hour=`echo ${hour}`
    fi

    if [ ${minute} -lt 10 ]; then
        str_minute=`echo 0${minute}`
    else
        str_minute=`echo ${minute}`
    fi

    if [ ${second} -lt 10 ]; then
        str_second=`echo 0${second}`
    else
        str_second=`echo ${second}`
    fi

    if [ ${ms} -lt 10 ]; then
        str_ms=`echo 00${ms}`
    elif [ ${ms} -lt 100 ]; then
        str_ms=`echo 0${ms}`
    else
        str_ms=`echo ${ms}`
    fi

    mv ${FILE} ${year}-${str_month}-${str_day}_${str_hour}-${str_minute}-${str_second}.${str_ms}.gif
done


# ========================== *.mp4 ==========================
ls *.mp4 > filenames.tmp

for FILE in `cat filenames.tmp | grep -v IMG`
do
    unix_time=`echo ${FILE} | cut -c 1-13`
    echo 'Unix time: '${unix_time}

    # convert UTC to UTC+8 (CST: China Standard Time)
    # 28800000 = 3600*8*1000
    unix_time=$(( ${unix_time} + 28800000 ))

    # There're 31536000000 miliseconds in a common year.
    # 31536000000 = 1000*86400*365
    year=$(( $((${unix_time} / 31536000000)) + 1970 ))

    # date: how many days since the first day in this year, *excluding* the current day!!
    # i.e. Jan. 1 --> 0
    #      Jan. 2 --> 1
    #      Feb. 1 --> 31
    # There're 86,400,000 miliseconds in a day.
    # $((${year} - 1972)) / 4: how many leap days since 1970, *excluding* the current year!!
    date=$(( ${unix_time} % 31536000000 / 86400000 - $((${year} - 1972)) / 4 ))
    if [ ${date} -lt 0 ]; then
        date=$(( ${date} + 365 ))
        year=$(( ${year} - 1 ))
    fi

    hour=$(( ${unix_time} % 86400000 / 3600000 ))
    minute=$(( ${unix_time} % 3600000 / 60000 ))
    second=$(( ${unix_time} % 60000 / 1000 ))
    ms=$(( ${unix_time} % 1000 ))

    # check if it's in a leap year
    if [ $((${year} % 4)) -eq 0 ]; then
        #echo 'leap year!'
        if [ ${date} -ge 335 ]; then
            month=12
            days_before_this_month=335
        elif [ ${date} -ge 305 ]; then
            month=11
            days_before_this_month=305
        elif [ ${date} -ge 274 ]; then
            month=10
            days_before_this_month=274
        elif [ ${date} -ge 244 ]; then
            month=9
            days_before_this_month=244
        elif [ ${date} -ge 213 ]; then
            month=8
            days_before_this_month=213
        elif [ ${date} -ge 182 ]; then
            month=7
            days_before_this_month=182
        elif [ ${date} -ge 152 ]; then
            month=6
            days_before_this_month=152
        elif [ ${date} -ge 121 ]; then
            month=5
            days_before_this_month=121
        elif [ ${date} -ge 91 ]; then
            month=4
            days_before_this_month=91
        elif [ ${date} -ge 60 ]; then
            month=3
            days_before_this_month=60
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi

    else
        #echo 'common year'
        if [ ${date} -ge 334 ]; then
            month=12
            days_before_this_month=334
        elif [ ${date} -ge 304 ]; then
            month=11
            days_before_this_month=304
        elif [ ${date} -ge 273 ]; then
            month=10
            days_before_this_month=273
        elif [ ${date} -ge 243 ]; then
            month=9
            days_before_this_month=243
        elif [ ${date} -ge 212 ]; then
            month=8
            days_before_this_month=212
        elif [ ${date} -ge 181 ]; then
            month=7
            days_before_this_month=181
        elif [ ${date} -ge 151 ]; then
            month=6
            days_before_this_month=151
        elif [ ${date} -ge 120 ]; then
            month=5
            days_before_this_month=120
        elif [ ${date} -ge 90 ]; then
            month=4
            days_before_this_month=90
        elif [ ${date} -ge 59 ]; then
            month=3
            days_before_this_month=59
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi
    fi

    day=$(( ${date} - ${days_before_this_month} + 1 ))

    # add leading zeros
    if [ ${month} -lt 10 ]; then
        str_month=`echo 0${month}`
    else
        str_month=`echo ${month}`
    fi

    if [ ${day} -lt 10 ]; then
        str_day=`echo 0${day}`
    else
        str_day=`echo ${day}`
    fi

    if [ ${hour} -lt 10 ]; then
        str_hour=`echo 0${hour}`
    else
        str_hour=`echo ${hour}`
    fi

    if [ ${minute} -lt 10 ]; then
        str_minute=`echo 0${minute}`
    else
        str_minute=`echo ${minute}`
    fi

    if [ ${second} -lt 10 ]; then
        str_second=`echo 0${second}`
    else
        str_second=`echo ${second}`
    fi

    if [ ${ms} -lt 10 ]; then
        str_ms=`echo 00${ms}`
    elif [ ${ms} -lt 100 ]; then
        str_ms=`echo 0${ms}`
    else
        str_ms=`echo ${ms}`
    fi

    mv ${FILE} ${year}-${str_month}-${str_day}_${str_hour}-${str_minute}-${str_second}.${str_ms}.mp4
done


# ========================== *.jpg/jpeg ==========================
ls *.jpg > filenames.tmp
ls *.jpeg >> filenames.tmp

for FILE in `cat filenames.tmp | grep -v IMG`
do
    unix_time=`echo ${FILE} | cut -c 1-13`
    echo 'Unix time: '${unix_time}

    # convert UTC to UTC+8 (CST: China Standard Time)
    # 28800000 = 3600*8*1000
    unix_time=$(( ${unix_time} + 28800000 ))

    # There're 31536000000 miliseconds in a common year.
    # 31536000000 = 1000*86400*365
    year=$(( $((${unix_time} / 31536000000)) + 1970 ))

    # date: how many days since the first day in this year, *excluding* the current day!!
    # i.e. Jan. 1 --> 0
    #      Jan. 2 --> 1
    #      Feb. 1 --> 31
    # There're 86,400,000 miliseconds in a day.
    # $((${year} - 1972)) / 4: how many leap days since 1970, *excluding* the current year!!
    date=$(( ${unix_time} % 31536000000 / 86400000 - $((${year} - 1972)) / 4 ))
    if [ ${date} -lt 0 ]; then
        date=$(( ${date} + 365 ))
        year=$(( ${year} - 1 ))
    fi

    hour=$(( ${unix_time} % 86400000 / 3600000 ))
    minute=$(( ${unix_time} % 3600000 / 60000 ))
    second=$(( ${unix_time} % 60000 / 1000 ))
    ms=$(( ${unix_time} % 1000 ))

    # check if it's in a leap year
    if [ $((${year} % 4)) -eq 0 ]; then
        #echo 'leap year!'
        if [ ${date} -ge 335 ]; then
            month=12
            days_before_this_month=335
        elif [ ${date} -ge 305 ]; then
            month=11
            days_before_this_month=305
        elif [ ${date} -ge 274 ]; then
            month=10
            days_before_this_month=274
        elif [ ${date} -ge 244 ]; then
            month=9
            days_before_this_month=244
        elif [ ${date} -ge 213 ]; then
            month=8
            days_before_this_month=213
        elif [ ${date} -ge 182 ]; then
            month=7
            days_before_this_month=182
        elif [ ${date} -ge 152 ]; then
            month=6
            days_before_this_month=152
        elif [ ${date} -ge 121 ]; then
            month=5
            days_before_this_month=121
        elif [ ${date} -ge 91 ]; then
            month=4
            days_before_this_month=91
        elif [ ${date} -ge 60 ]; then
            month=3
            days_before_this_month=60
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi

    else
        #echo 'common year'
        if [ ${date} -ge 334 ]; then
            month=12
            days_before_this_month=334
        elif [ ${date} -ge 304 ]; then
            month=11
            days_before_this_month=304
        elif [ ${date} -ge 273 ]; then
            month=10
            days_before_this_month=273
        elif [ ${date} -ge 243 ]; then
            month=9
            days_before_this_month=243
        elif [ ${date} -ge 212 ]; then
            month=8
            days_before_this_month=212
        elif [ ${date} -ge 181 ]; then
            month=7
            days_before_this_month=181
        elif [ ${date} -ge 151 ]; then
            month=6
            days_before_this_month=151
        elif [ ${date} -ge 120 ]; then
            month=5
            days_before_this_month=120
        elif [ ${date} -ge 90 ]; then
            month=4
            days_before_this_month=90
        elif [ ${date} -ge 59 ]; then
            month=3
            days_before_this_month=59
        elif [ ${date} -ge 31 ]; then
            month=2
            days_before_this_month=31
        else
            month=1
            days_before_this_month=0
        fi
    fi

    day=$(( ${date} - ${days_before_this_month} + 1 ))

    # add leading zeros
    if [ ${month} -lt 10 ]; then
        str_month=`echo 0${month}`
    else
        str_month=`echo ${month}`
    fi

    if [ ${day} -lt 10 ]; then
        str_day=`echo 0${day}`
    else
        str_day=`echo ${day}`
    fi

    if [ ${hour} -lt 10 ]; then
        str_hour=`echo 0${hour}`
    else
        str_hour=`echo ${hour}`
    fi

    if [ ${minute} -lt 10 ]; then
        str_minute=`echo 0${minute}`
    else
        str_minute=`echo ${minute}`
    fi

    if [ ${second} -lt 10 ]; then
        str_second=`echo 0${second}`
    else
        str_second=`echo ${second}`
    fi

    if [ ${ms} -lt 10 ]; then
        str_ms=`echo 00${ms}`
    elif [ ${ms} -lt 100 ]; then
        str_ms=`echo 0${ms}`
    else
        str_ms=`echo ${ms}`
    fi

    mv ${FILE} ${year}-${str_month}-${str_day}_${str_hour}-${str_minute}-${str_second}.${str_ms}.jpg
done


rm -f filenames.tmp
