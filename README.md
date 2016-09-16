# UPEI Roblib Reports

## Introduction

A module to track space used by a site, including
* drupal database size
* local drupal disk usage (public/private files)
* Islandora/Fedora storage.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/tuque)
* [Islandora Solr](https://github.com/Islandora/islandora_solr_search)

## Installation

Install as for a usual Drupal module. Before the first report has run, all disk usage reports will read zero.

## Configuration

Configure this module at Islandora > Islandora Utility Modules > Roblib Reports Config (admin/islandora/tools/roblib/reports).

Because the measurement script can expensive to run and is predicted to slow down
the connected repository, it may not be necessary to run more than once every few days.

## Usage

Reports can be found at Administer > Reports > Islandora Disk Usage Report. Reports run during cron, at the frequency set
in the configuration, above. To refresh the reports, either run cron or use the drush script below.

Drush:

`$ drush upei-roblib-reports-run`

returns:

 `UPEI Roblib Report generated in 19 sec. Peak memory usage: 63 MB.`

## Maintainers/Sponsors

Current maintainers:

* [Paul Pound](https://github.com/ppound)

## License

No license applied yet.