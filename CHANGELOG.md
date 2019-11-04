# Changelog

Notable changes to Comfy will be documented in this log.

## 1.0.1 - 2019-11-04

### Added

- MultiChildWidget class to serve as a base class for widgets that contain multiple children, such as HorizontalBoxWidgets and VerticalBoxWidgets.

### Changed

- Threads and catalogs can no longer be manually reloaded with CTRL+R if they are already in the processs of reloading (waiting for a the result of a NetOps operation). This prevents the NetOps queue from being flooded with get requests when network speeds are slow, or the network connection is lost or unstable.

- Imageboard files (json, images etc.) are now stored in $HOME/.comfy/imageboards/, and unsaved files are now only automatically deleted on application exit from that directory and $HOME/.comfy/tmp (instead of the all unsaved files in $HOME/.comfy/).

- Added logic to prevent a widget's child or sub-child widgets from being set as a widget's parent, and logic that prevents a widget's parent or super-parent from being set as a widget's child.

