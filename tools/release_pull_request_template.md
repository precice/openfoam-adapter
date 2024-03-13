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

## Preparing the Changelog

- [ ] Copy all the changelog entries into `CHANGELOG.md`
- [ ] Delete `changelog-entries/`
- [ ] Mark issues and pull requests that made it to this milestone.
- [ ] Collect a list of contributors to acknowledge (particularly non-code contributions that are not easily visible on GitHub).
- [ ] Draft and discuss release notes

## Bump the version

- [ ] Decide on the version using the versioning strategy documented in the user docs (see page "Get the OpenFOAM adapter")
- [ ] Bump the version in `CHANGELOG.md`
- [ ] Bump the version in `Adapter.C` (removing the `+ unreleased changes` part).
- [ ] Make the version bump commit the last commit before merging.

## Merge

- [ ] Review pull request
- [ ] Merge pull request (**not** squash)

## Prepare version-specific branches

- [ ] Rebase `OpenFOAM4` on `master` and force-push
  - `git checkout OpenFOAM4 && git rebase develop`
  - Resolve any conflicts. We should have only one commit in the end.
  - `git push --force`
- [ ] Rebase `OpenFOAM5` on `OpenFOAM4`, `OpenFOAM6` on `OpenFOAM5`, ...
- [ ] Trigger a custom build for each version and tick each when it succeeds:
  - [ ] OpenFOAM5 on Ubuntu 18.04 with preCICE 2.3.0 and tutorials from develop
  - [ ] OpenFOAM6 on Ubuntu 18.04 with preCICE 2.3.0 and tutorials from develop
  - [ ] OpenFOAM7 on Ubuntu 20.04 with preCICE latest and tutorials from develop
  - [ ] OpenFOAM8 on Ubuntu 20.04 with preCICE latest and tutorials from develop
  - [ ] OpenFOAM9 on Ubuntu 20.04 with preCICE latest and tutorials from develop
  - [ ] OpenFOAM10 on Ubuntu 20.04 with preCICE latest and tutorials from OpenFOAM10
  - [ ] OpenFOAM v1912 (adapter master) on Ubuntu 18.04 with preCICE v2.3.0 and tutorials from develop

Overview of branches:

```text
master <-- OpenFOAM4 <-- OpenFOAM5 <-- OpenFOAM6 <-- OpenFOAM7 <-- ... <-- OpenFOAMdev
^-- develop     ^-- OpenFOAMv1806
```

## Release

- [ ] Create a Release on GitHub
- [ ] Download an archive of the repository (i.e., no `.git/` or `.gitignore`) for each version and attach to the release. Use the link `https://github.com/precice/openfoam-adapter/archive/refs/heads/BRANCH.tar.gz` (substitute `BRANCH`). Rename the folder in each archive to reflect the name of the archive.
  - branch `master`: archive `openfoam-adapter_v1.0.0_OpenFOAMv1812-v2206-newer.tar.gz` (adjust `v2206` to the latest supported, and `v1.0.0` to the actual version)
  - branch `OpenFOAM4`: archive `openfoam-adapter_v1.0.0_OpenFOAM4_5_v1806.tar.gz`
  - branch `OpenFOAM6`: archive `openfoam-adapter_v1.0.0_OpenFOAM6_experimental.tar.gz`
  - branch `OpenFOAM7`: archive `openfoam-adapter_v1.0.0_OpenFOAM7_experimental.tar.gz`
  - branch `OpenFOAM8`: archive `openfoam-adapter_v1.0.0_OpenFOAM8_experimental.tar.gz`

## Post-release

- [ ] Merge back from `master` to `develop`. No PR is needed for that.
- [ ] Modify the adapter version message to `Loaded the OpenFOAM-preCICE adapter v1.0.0 + unreleased changes`.
- [ ] Update workflows in the tutorials repository, if needed (e.g., OpenFOAM version)
- [ ] Update external documentation (tutorials, website), e.g., regarding the adapter or OpenFOAM version.
  - [ ] Quickstart
- [ ] Update the VM provisioning scripts, if needed (e.g., OpenFOAM version)
- [ ] Update the [Spack package](https://github.com/kjrstory/spack/blob/develop/var/spack/repos/builtin/packages/of-precice/package.py)
- [ ] Update this release checklist (`tools/release_pull_request_template.md`)
- [ ] Advertise and celebrate! :tada: :beers:
