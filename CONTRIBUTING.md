# Contributing to the OpenFOAM-preCICE adapter

Hi there! Amazing that you want to contribute to this project!
Please have a look at the page [contributing to preCICE](https://precice.org/community-contribute-to-precice.html)
and watch out for more specific details in this file.

## Changelog

Instead of directly editing `CHANGELOG.md`, please add a file `123.md`
in `changelog-entries`, where `123` your pull request number. This helps reduce
merge conflicts and we will merge these files at the time we release a new version.

## Code formatting

We use [pre-commit](https://pre-commit.com/) to ensure consistent formatting.
Please install `pre-commit` and then install the hook in this repository with `pre-commit install`.
This ensures correct formatting for future commits.
Run `pre-commit run -va` to apply formatting retrospectively.

## Automatic checks

We check every contribution with a few GitHub Actions workflows that report at the bottom of each pull request.

There are also a few additional workflows that can be triggered manually:

- `Custom build`: builds any branch of the repository with any of the available OpenFOAM versions.
- `Check links`: checks the links in all markdown files to verify if they are still reachable.

Members of the repository can trigger these workflows in the "Actions" tab.

### Running the CI locally

You can also run GitHub Actions locally with [act](https://github.com/nektos/act).

List the workflows with `act --list`. To simulate a push event, run `act push`. Some workflows might work on GitHub Actions but not on act, in which case you could try running specific workflows with, for example, `act -j 'build'`.

To trigger the custom build workflow:

1. Switch to the directory `.github/workflows/`
2. Edit the inputs in the `build-custom.input` file.
3. [Generate a GitHub access token](https://github.com/settings/personal-access-tokens) and add it to a `.secrets` file with content `GITHUB_TOKEN=<your-token>`.
   This, as well as setting an `--artifact-server-path` to a local directory are needed for the `upload-artifact` action, even though build artifacts are only stored locally (see updates in a [related issue](https://github.com/nektos/act/issues/329)).
4. Start the build using:

   ```shell
   act -W build-custom.yml --input-file build-custom.input --secret-file .secrets --artifact-server-path $PWD/.artifacts
   ```

Find valid combinations of Ubuntu and OpenFOAM versions in the [OpenFOAM support page](https://precice.org/adapter-openfoam-support.html).

## System tests

For non-trivial pull requests, we also need to execute [system regression tests](https://precice.org/dev-docs-system-tests.html),
to ensure that complete simulations still run and give the same results.
Because these take long, run on an external system, and consume significant resources,
we are only triggering these on demand. Add (or ask a maintainer to add) the
`trigger-system-tests` label to the pull request to trigger them.
The tests will only run once, so that further commits don't consume additional
resources: In case you want to re-trigger them, remove and add the label again.
