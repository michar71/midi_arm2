import com.bckmnn.json.*;

try{
  JSONObject j = loadJSONObject("http://maps.googleapis.com/maps/api/geocode/json?address=1600+Amphitheatre+Parkway,+Mountain+View,+CA&sensor=true");
  println(JSONHelper.getString("results[0].address_components[0].long_name",j));
  println(JSONHelper.getString("results[0].formatted_address",j));  
  println(JSONHelper.getFloat("results[0].geometry.location.lat",j));
  println(JSONHelper.getFloat("results[0].geometry.location.lng",j));
  println(JSONHelper.getString("results[0].types[0]",j)); 
}catch (Exception e){
  println("oops " + e);
}


