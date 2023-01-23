
import os

Import("projenv", "env")

print(os.environ.items())


def get_env_data_as_dict(dotenv_path):
    # thanks to https://stackoverflow.com/a/74856258/3631348
    # have to parse manually since dotenv package is not available
    ''' read file and parse key/values '''

    try:
        with open(dotenv_path, 'r', encoding='utf8') as file_obj:
            lines = file_obj.read().splitlines()  # Removes \n from lines
    except:
        return {}

    result = {}
    for line in lines:
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue

        if "#" in line:
            line = line.split("#")[0].strip()

        k, v = line.split("=", maxsplit=1)
        result[k] = v

    return result


dotenv = get_env_data_as_dict(os.getcwd()+'/.env')


def parse_github_envs():
    ''' read github envs, prefixed with "SMAHOO_" .. '''

    for key, val in os.environ.items():
        if not key.startswith('SMAHOO_'):
            continue

        dotenv[key.replace('SMAHOO_', '', 1)] = val


parse_github_envs()


def patch_into_cppdefs():
    ''' patch collected settings into CPPDEF's '''
    for key, val in dotenv.items():
        # patch every key and value into the projects CPP defs..
        projenv.Append(CPPDEFINES=[
            (key, env.StringifyMacro(val))
        ])
