<?php

/**
 * @file
 * Holds the GBIF api callbacks for the islandora_biostats.
 */

/**
 * Gbif API main callback function
 * @param $apiurl
 * string, the path or specific API rest method we are calling, always starts with "/"
 * @param $args
 * associative array containing the arguments we are passing to gbif
 * @return a full associative array with data to be displayed or consumed by other functions decoded from GBIF API JSON response
*/
function islandora_biostats_gbifapi_getcontent($apiurl, $args)
{
 $gbif_base = "http://api.gbif.org/v1";
   $url = $gbif_base.$apiurl."?".http_build_query($args);
  
   $httpresponse = drupal_http_request($url);
     if (!empty($response->error)) {
       $msg_args = array(
         '%url' => $url,
         '@code' => $httpresponse->code,
         '%error' => $httpresponse->error,
       );
       watchdog_exception(t('Biostats, the HTTP request %url returned the following error (code @code): %error.'), $msg_args, WATCHDOG_ERROR);
       return FALSE;
     }
     $response = drupal_json_decode($httpresponse->data);//returns an array
     if (is_array($response)) {return $response;} //GBIF API also returns integers and strings..not only json
     else {
       return $httpresponse->data;
     }
     
}

function islandora_biostats_gbifapi_count($taxonkey)
{
  $apiurl = "/occurrence/count";
  //Note the order is important! Never new...now i know.
  $args = array(
    'isGeoreferenced' =>'true',
    'country' => 'CL',  
    'taxonKey' => $taxonkey,
   
  );
  $response=islandora_biostats_gbifapi_getcontent($apiurl,$args);
  return $response;
}


function islandora_biostats_gbifapi_taxonkey($scientificname)
{
  $apiurl = "/species/match";
  $args = array(
   'strict' => 'true',  
   'verbose'=> 'true',
   'name'=> trim($scientificname), 
    
  );
  $response=islandora_biostats_gbifapi_getcontent($apiurl,$args);
  return $response;
}
/**
 * GBif get ocurrences based on arguments
 * @param $apiurl
 * string, the path or specific API rest method we are calling, always starts with "/"
 * @param $args
 * associative array containing allowed GBIF 1.0 API arguments for ocurrences/search endpoint http://www.gbif.org/developer/occurrence
 * @return a full associative array with data to be displayed or consumed by other functions decoded from GBIF API JSON response. 
 * In this case the we get something like this. We can use this to calculate offsets and pagination.

 *     "count": 11, 
 *     "endOfRecords": true, 
 *     "limit": 20, 
 *    "offset": 0,
 * "result": list of records... 
*/
function islandora_biostats_gbifapi_ocurrences(array $args)
{
  $apiurl = "/occurrence/search";
  $response = array();
  if (isset($args['offset']) && ($args['offset']>200000)) { //This is the hardcoded maximum GBIF will give us using this endpoint
    return $response;
  }
  if ((count($args)>0) && isset($args['country'])) //We only check if country is given, this is a enough limit for us. There is a 300 limit per page, we are using this
    {
    $response=islandora_biostats_gbifapi_getcontent($apiurl,$args);
    }
  return $response;
}

