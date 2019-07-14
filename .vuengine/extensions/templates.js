function main() 
{
    writeConfigH();
    writeConfigMake();
    writeFontFiles();
}

function writeConfigH()
{
    let folder = vuengine.getWorkspaceRoot();

    let template = 'config.h';
    let filename = folder + '/source/' + template;
    let settings = vuengine.getProjectConfig('settings');
    let romHeader = vuengine.getProjectConfig('romHeader');

	let data = vuengine.mergeRecursively(settings, romHeader, {
        "data": {
            "pluginsConfigFileExists": fs.existsSync(folder + '/source/pluginsConfig.h')
        }
    });

    // convert game-specific values to upper snake case
    let newGameMacros = {};
    for (let key of Object.keys(data['game'])) {
        let convertedKey = vuengine.toUpperSnakeCase(key);
        let convertedValue = data['game'][key];
        if (typeof convertedValue === 'string' || convertedValue instanceof String) {
            convertedValue = '"' + convertedValue + '"';
        }

        newGameMacros[convertedKey] = convertedValue;
    }
    data['game'] = newGameMacros;

    templates.renderToFile(__dirname + '/templates/' + template + '.nj', filename, data);
}

function writeConfigMake()
{
    let folder = vuengine.getWorkspaceRoot();

    let template = 'config.make';
    let filename = folder + '/' + template;
	let settings = vuengine.getProjectConfig('settings');
	let plugins = vuengine.getProjectConfig('plugins');
    let data = vuengine.mergeRecursively(settings, plugins);

    templates.renderToFile(__dirname + '/templates/' + template + '.nj', filename, data);
}


function writeFontFiles()
{
    let folder = vuengine.getWorkspaceRoot();

    let fonts = vuengine.getProjectConfig('fonts');

    templates.renderToFile(
        __dirname + '/templates/Fonts.c.nj',
        folder + '/assets/fonts/Fonts.c',
        fonts
    );

    templates.renderToFile(
        __dirname + '/templates/Fonts.h.nj',
        folder + '/source/assets/Fonts.h',
        fonts
    );
}
