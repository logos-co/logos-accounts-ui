{
  description = "Accounts interface for the Logos application";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder";
    accounts_module.url = "github:logos-co/logos-accounts-module";
  };

  outputs = inputs@{ logos-module-builder, ... }:
    logos-module-builder.lib.mkLogosQmlModule {
      src = ./.;
      configFile = ./metadata.json;
      flakeInputs = inputs;
    };
}
