<?php

/**
 * @file
 * Holds the basic GBIF Occurrences Object classes for islandora_biostats.
 * This class allows to store(cache), fetch and process statistical data from GBIF
*/


/**
 * GBIF occurrence Records Class
 */
class biostatsGBIFOccurrencesStats {

  /**
   * GBIF taxonKey.
   *
  * @var taxonKey
  */
 protected $taxonKey;
 /**
  * GBIF occurrences count.
  *
 * @var taxonKey
 */
 protected $expectedcount; //This is the occurrences count in GBIF, not in our object
 protected $locallystored; //This is the occurrences count in our object
 protected $lastUpdate;
 private $gbifocurrences; //JSON format
 //protected $lastGbifUpdate;
 protected $facets;
  


public function __construct($taxonKey,$expectedcount) {
  $this->taxonKey = $taxonKey;
  $this->expectedcount = $expectedcount;
  $this->facets=array(); //empty
  $this->locallystored=0;
  $this->gbifocurrences=json_encode(array());
}
 
 
 public function addJSon(array $chunk,$offset) {
    //We only can add/replace if offset is <=locallystored, so we maintain our sequential GBIF order
   if ($offset <= $this->locallystored) {
     if (!$this->_areNewRecords($offset)) {
       $newArray = $chunk;
       $newRecords= count($newArray);
       $this->_facet($newArray);
       
       $currentArray = json_decode($this->getJson(),true);
       $updatedArray=array_merge($currentArray,$newArray);
       $this->_setJson(json_encode($updatedArray));
       $this->locallystored=count($updatedArray);
      
       return true;
     }
     else { //means we are replacing, we need to slice the original one and append our new data
       $newArray = $chunk;
       $this->_facet($newArray);
       $currentArray = json_decode($this->getJson(),true);
       $newcurrentArray = array_slice($currentArray, 0, $offset);
       $updatedArray=array_merge($newcurrentArray,$newArray);
       $this->_setJson(json_encode($updatedArray));

       $this->locallystored=count($updatedArray);
       return true;
     } 
   }
   return false;
 }
 public function getOffset() {
   return $this->locallystored;
 }
 public function isPartial() {
   return ($this->locallystored < $this->expectedcount);
 }
public function getJson() {
  return $this->gbifocurrences;
}
public function getFacets() {
  return $this->facets;
}

 private function _areNewRecords($offset) {
   if ($this->locallystored >= $offset) {//means we are overwriting, because offset is our starting record+recordcount
     return false;
   } 
   else {
     return true;
   }
 }
 private function _setJson($json) {
   $this->gbifocurrences=$json;
   $this->lastUpdate = new DateTime('now');
 }
 private function _setFacet($count) {
   $this->facets=$count; //@TODO sum values of existing facets
  
 }
 private function _facet(array $chunkArray) {
   $reduced = array_walk($chunkArray,array($this, '_reduceToKeys'));
   if ($reduced) {
   $count = call_user_func_array('array_merge_recursive', $chunkArray);
   array_walk($count,array($this, '_countByKeys'));
   $this->_setFacet($count);
   }
 }
 private function _reduceToKeys(&$chunkArrayRecord) {
   $facets = array('datasetKey','dept','elevation','year','month','publishingCountry','publishingOrgKey','institutionCode','collectionCode','basisOfRecord');
   $chunkArrayRecord = array_intersect_key($chunkArrayRecord, array_flip($facets)); 
 }
 private function _countByKeys(&$chunkArrayRecord) {
   
   $chunkArrayRecord = array_count_values($chunkArrayRecord); 
 }
   
 
}
