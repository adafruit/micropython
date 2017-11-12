Feature: Help information
  Scenario: User types --help
    Given user types 'nrfutil dfu genpkg --help'
    When user press enter
    Then output contains 'Generate a zipfile package for distribution to Apps supporting Nordic DFU' and exit code is 0

  Scenario: User does not type mandatory arguments
    Given user types 'nrfutil dfu genpkg'
    When user press enter
    Then output contains 'Error: Missing argument "zipfile".' and exit code is 2
