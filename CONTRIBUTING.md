# Contributing to the OpenFOAM-preCICE adapter

Hi there! Amazing that you want to contribute to this project!
Please have a look at the page [contributing to preCICE](https://precice.org/community-contribute-to-precice.html)
and watch out for more specific details in this file.

## Changelog

Instead of directly editing `CHANGELOG.md`, please add a file `123.md`
in `changelog-entries`, where `123` your pull request number. This helps reduce
merge conflicts and we will merge these files at the time we release a new version.

## Code formatting

We use `pre-commit` to ensure consistent formatting.
Please install `pre-commit` and then install the hook in the repo `pre-commit install`.
This ensures correct formatting for future commits.
Run `pre-commit run -va` to apply formatting retrospectively.

## Automatic checks

We check every contribution with a few GitHub Actions workflows that report at the bottom of each pull request.

There are also a few additional workflows that can be triggered manually:

- `Custom build`: builds any branch of the repository with any of the available OpenFOAM versions.
- `Check links`: checks the links in all markdown files to verify if they are still reachable.

Members of the repository can trigger these workflows in the "Actions" tab.
