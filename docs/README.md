This branch is used only for the website under https://ebu.github.io/ear-production-suite/.
It is deployed automatically by Github Pages.

## Build locally

Requires Ruby install (installer contains Bundler)

1. `git clone -b gh-pages https://github.com/ebu/ear-production-suite.git`
2. `cd ear-production-suite`
3. Configure the `_config.yml` file as needed
4. `cd docs`
5. `bundle install`
6. `bundle exec jekyll serve`
