# Release checklist

Copy this template to the release pull request description.

## Update workflows

- [ ] `.github/workflows/build.yml` and `.github/workflows/install-dependencies.sh`: Update the OpenFOAM and preCICE versions, if needed.
- [ ] `build-custom.yml`: Add any new OpenFOAM versions and update the default preCICE version, if needed.

## Prepare branches

- [ ] Merge `master` to `develop`, if needed
- [ ] Open a pull request from `develop` to `master`.
  - We don't use release branches, unless there are changes we don't want to release, potentially when preparing a major release.
- [ ] Run and fix all workflows

## Test

- [ ] Run and check flow-over-heated-plate OpenFOAM-OpenFOAM
- [ ] Run and check quickstart
- [ ] Run and check perpendicular-flap OpenFOAM-deal.II
- [ ] Run and check elastic-tube-3d OpenFOAM-CalculiX
- [ ] Run and check partitioned-pipe sonicLiquidFoam-sonicLiquidFoam
- [ ] Run and check heat-exchanger OpenFOAM-CalculiX-OpenFOAM

Until we get automated system tests again, run all the tests manually.

## Prepare version-specific branches

- [ ] Rebase `OpenFOAM4` on `develop` and force-push
  - `git checkout OpenFOAM4 && git rebase develop`
  - Resolve any conflicts. We should have only one commit in the end.
  - `git push --force`
- [ ] Rebase `OpenFOAM5` on `OpenFOAM4`, `OpenFOAM6` on `OpenFOAM5`, ...
- [ ] Trigger a custom build for each version

Overview of branches:

```text
master <-- OpenFOAM4 <-- OpenFOAM5 <-- OpenFOAM6 <-- OpenFOAM7 <-- OpenFOAM8 <-- OpenFOAMdev
^-- develop     ^-- OpenFOAMv1806
```

## Preparing the Changelog

- [ ] Copy all the changelog entries into `CHANGELOG.md`
- [ ] Delete `changelog-entries/`
- [ ] Draft and discuss release notes

## Bump the version

- [ ] Decide on the version using the versioning strategy documented in the user docs (see page "Get the OpenFOAM adapter")
- [ ] Bump the version in `CHANGELOG.md`
- [ ] Bump the version in `Adapter.C` (removing the `+ unreleased changes` part).

## Merge

- [ ] Review pull request
- [ ] Merge pull request (**not** squash)
- [ ] Update the version in `Adapter.C` on `develop` to reflect that this is an unreleased version
- [ ] Rebase the version-specific branches on `master`

## Release

- [ ] Create a Release on GitHub
- [ ] Download an archive of the repository (i.e., no `.git/` or `.gitignore`) for each version and attach to the release
  - branch `master`: archive `openfoam-adapter_v1.0.0_OpenFOAMv1812-v2112.tar.gz` (adjust `v2112` to the latest supported, and `v1.0.0` to the actual version)
  - branch `OpenFOAM4`: archive `openfoam-adapter_v1.0.0_OpenFOAM4_5_v1806.tar.gz`
  - branch `OpenFOAM6`: archive `openfoam-adapter_v1.0.0_OpenFOAM6_experimental.tar.gz`
  - branch `OpenFOAM7`: archive `openfoam-adapter_v1.0.0_OpenFOAM7_experimental.tar.gz`
  - branch `OpenFOAM8`: archive `openfoam-adapter_v1.0.0_OpenFOAM8_experimental.tar.gz`

## Post-release

- [ ] Merge back from `master` to `develop`. No PR is needed for that.
- [ ] Modify the adapter version message to `Loaded the OpenFOAM-preCICE adapter v1.0.0 + unreleased changes`.
- [ ] Update the git module on the website
- [ ] Update workflows in the tutorials repository, if needed (e.g., OpenFOAM version)
- [ ] Update the VM provisioning, if needed (e.g., OpenFOAM version)
- [ ] Update this release checklist (`tools/release_pull_request_template.md`)
- [ ] Advertise and celebrate! :tada: :beers:
