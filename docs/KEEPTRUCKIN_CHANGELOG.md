# KeepTruckin rgeo-proj4 changelog

## rails7-upgrade

Base
- Upstream rgeo-proj4 4.0.0 (PROJ 6+ API, uses `proj.h`).
- Upstream rgeo 3.1.x requirement (aligns with Rails 7.2 track).

KeepTruckin custom layers
- Restore file-based SRS database reader `RGeo::CoordSys::SRSDatabase::Proj4Data`.
- Allow disabling default header/lib search paths in `extconf.rb` via:
  - `--without-default-header-paths`
  - `--without-default-lib-paths`

Notes
- The custom changes are minimal and sit on top of upstream v4 API behavior.
- No deprecated `proj_api.h` compatibility is reintroduced in this branch.
