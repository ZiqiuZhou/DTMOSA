#!/bin/sh

loc_file="/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_need_predict_loc.json"
loc_file_after_NER="/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_after_NER_predict.json"
loc_file_after_address_net="/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_after_address_net_predict.json"
loc_file_after_location_predict="/home/dietrich/master_thesis/GeoBurst_OSM/data/tweets_after_location_prediction.json"

if [ -f"$loc_file_after_NER" ];then
    rm -f $loc_file_after_NER
    touch $loc_file_after_NER
fi

python3 location_predict_NER.py

if [ $?==0 ];then
    echo "NER location prediction finished."
else
    echo "NER prediction exception occurs."
    exit 1
fi

if [ $?==0 ];then
    if [ -f"$loc_file_after_address_net" ];then
        rm -f $loc_file_after_address_net
        touch $loc_file_after_address_net
    fi

    python2 location_predict_address_net.py

    if [ $?==0 ];then
        echo "address location prediction finished."
    else
        echo "address prediction exception occurs."
        exit 1
    fi
fi

if [ $?==0 ];then
    if [ -f"$loc_file_after_location_predict" ];then
        rm -f $loc_file_after_location_predict
        touch $loc_file_after_location_predict
    fi

    python location_prediction.py

    if [ $?==0 ];then
        echo "address location prediction finished."
    else
        echo "address prediction exception occurs."
        exit 1
    fi
fi
