Biostats Solution Pack for Islandora 7.1-x (Tested under 1.4 and 1.5)
------------

Requires Biodiversity Solution Pack and Ontology Solution Pack
---------------------------------------------------------------------------------

This Solution Pack adds a Statistic/Comparative View to every Islandora Biodiversity CMODEL Object based on Occurrences for Chile(can be customized).

Using the existing data/solr/objects/relations provided by the repository it fetches GBIF Occurrence data from GBIF API 1.0 by generating a streamed JSON (continuos) feed that is read and
converted to cacheable objects. Local Solr and SPARQL data is then compared. It also provides an JS Abstraction to Google Charts and additional Leaflet progressive JSON reader to allow to display and compute
partial/progressive streamed data using oboe.js.

This is a developer version, in progress, not suitable to production (still) but going there fast, very fast.

## Features
 * JSON streaming of live and Cached GBIF Occurrence data 
 * Incremental population of Objects by TaxonKey. So we can serve data in a mix of live and cached
 * Incremental faceting for useful data. Various statistical coeficients are calculated/faceted/incremented in realtime(or at least near)
 * In case a taxon is not present in GBIF (and we got it here) it searches the taxonomic hierachy up, allowing comparative agains the first matched simil.
 * Leaflet + streaming JSON can manage over 50.000 points (using markerclustering)
## @TODO

 * Cleanup
 * Theming
 * Displaying all our stats in graphs, we have the data, just put them there
 * Display associated species in a given radius(km)
 * Optimize Cache/test alternatives (like adding to Solr too)

## Requirements

Good, nice curated data.

This module requires also the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [Islandora Solr](https://github.com/islandora/islandora_solr_search/)
* [Islandora Biodiversity Solution Pack] 
* [Islandora Ontologies]

## Maintainers/Sponsors

Current maintainers:

* [Diego Pino](https://github.com/DiegoPino)

## License

(as always)

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)