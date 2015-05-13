<?php

/**
 * @file
 * Holds the GBIF api callbacks for the islandora_biostats.
 */

/**
 * Gbif API main callback function
 * @param $apiurl
 * string, the path or specific API rest method we are calling, always starts with "/"
 * @param $query_processor
 * An IslandoraSolrQueryProcessor with q, fl, etc
 * @return a Solr response,  with results
*/
function islandora_biostats_redbioapi_getcontent_solr($query_processor)
{
 
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $response = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Biostats', $e, 'Got an exception while searching in Solr ', array(), WATCHDOG_ERROR);
  }
  return $response;
}


function islandora_biostats_redbioapi_perobject_solr($pid)
{
  module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
 
  //In case we have the solr module enabled, check for the PID prefix
  if (module_exists('islandora_solrcloud_config')) 
    {
    //add the prefix  
    $query_field= variable_get('islandora_solrcloud_setting_uniqueid', 'PID');
    $query_token = variable_get('islandora_solrcloud_setting_uniqueid_prefix','').$pid;  
    }
  else {
    $query_field='PID';
    $query_token = $pid;
  }    
      
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = format_string(
    '!biodiversidad_query_field:("!biodiversidad_query")', array(
      '!biodiversidad_query_field' => $query_field,
      '!biodiversidad_query' => $query_token,
    )
  );
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 1;
  $xacml_restrictions=islandora_red_biodiversidad_getxacml_solrquery_restrictions();

  $query_processor->solrParams = array(
    'fl' => "dwc.*_s,dcterms.source_s,fgs_label_s,PID_fullurl",
    'fq' => "",
    );
    
        
  if (!empty($xacml_restrictions)) {
    //$query_processor->solrParams['fq'] = $query_processor->solrParams['fq']." AND ( ".$xacml_restrictions." )";
    $query_processor->solrParams['fq'] = "( ".$xacml_restrictions." )";
  }
  $response=islandora_biostats_redbioapi_getcontent_solr($query_processor);
  return $response;
}
function islandora_biostats_redbioapi_relatedtoobject_solr($pid, $solrrelations)
{
  module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
 
  //In case we have the solr module enabled, check for the PID prefix
  
  if (count($solrrelations)==0) {
    return array();
    }
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->buildQuery("*:*");
  $query_processor->solrParams = array(
    'fl' => "dwc.*_s,dcterms.source_s,fgs_label_s,PID_fullurl",
  );
  $filters=array();
  
  //Build the first !join
  $direct_rels=array_shift($solrrelations);
  
  $luceneReservedCharacters = preg_quote('+-&|!(){}[]^"~*?:\\');
    //Not nice: we are using a nested query so we need to double escape our only value, the famouse PID
  $pid= preg_replace_callback(
        '/([' . $luceneReservedCharacters . '])/',
        function($matches) {
            return '\\' . $matches[0];
        },
        $pid);
  $filters[] = format_string('{!join from=!direct_rels to=PID_uri}PID:!pid', array(
      '!direct_rels' => $direct_rels,
      '!pid' => islandora_solr_lesser_escape($pid),
    ));
  
  //*:* AND _query_:"{!join to=PID_uri from=RELS_EXT_identifies_uri_ms}PID_uri:* AND _query_:{!join from=RELS_EXT_taxonOfId_uri_ms to=PID_uri}PID:umce\\:a79c6f05\-87ce\-49e3\-b01a\-b4ac6e15ecba"*:*
  foreach($solrrelations as $rels_ext_field) {
    
    $filters[] = format_string('{!join to=PID_uri from=!rels_ext_field}PID_uri:*', array(
        '!rels_ext_field' => $rels_ext_field,
      ));
  }
  //$filters[]='*:*';
  $filters_string=implode(' AND _query_:',array_reverse($filters));
  $query_processor->solrParams['fq'] = '*:* AND _query_:"'.$filters_string.'"';
  $xacml_restrictions=islandora_red_biodiversidad_getxacml_solrquery_restrictions();
  if (!empty($xacml_restrictions)) {
  $query_processor->solrParams['fq'] = $query_processor->solrParams['fq']." AND ( ".$xacml_restrictions." )";
  }
  
  $response=islandora_biostats_redbioapi_getcontent_solr($query_processor);
  return $response;
}


function islandora_biostats_redbioapi_relatedtoobject_sparql($pid, $relations)
{
  module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
 
  //In case we have the solr module enabled, check for the PID prefix
  if (module_exists('islandora_solrcloud_config')) 
    {
    //add the prefix  
    $query_field= variable_get('islandora_solrcloud_setting_uniqueid', 'PID');
    $query_token = variable_get('islandora_solrcloud_setting_uniqueid_prefix','').$pid;  
    }
  else {
    $query_field='PID';
    $query_token = $pid;
  }    
      
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = format_string(
    '!biodiversidad_query_field:("!biodiversidad_query")', array(
      '!biodiversidad_query_field' => $query_field,
      '!biodiversidad_query' => $query_token,
    )
  );
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 1;
  $xacml_restrictions=islandora_red_biodiversidad_getxacml_solrquery_restrictions();

  $query_processor->solrParams = array(
    'fl' => "dwc.*_s,dcterms.source_s,fgs_label_s,PID_fullurl",
    'fq' => "",
    );
    
        
  if (!empty($xacml_restrictions)) {
    //$query_processor->solrParams['fq'] = $query_processor->solrParams['fq']." AND ( ".$xacml_restrictions." )";
    $query_processor->solrParams['fq'] = "( ".$xacml_restrictions." )";
  }
  
  $response=islandora_biostats_redbioapi_getcontent($query_processor);
  return $response;
}





