### Documentation
In this directory we mainly focus on predicting (recognize + geo-parse) two types of locations mentioned in tweet content.

"location_predict_NER.py" recognizes name related locations, such as the name or name descriptions of a place or an orgnization; "location_predict_address_net.py" recognizes address related locations, such as the address of a house. "location_prediction.py" predicts the recognized locations using geo-coding.

You can run the shell script "location_predict.sh" to finish the above tasks at one time, or you can run each routine separately.
