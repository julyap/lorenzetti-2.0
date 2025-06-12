#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int create_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        return mkdir(path, 0700);
    }
    return 0;
}

json_t* build_stages(const char *particle_name) {
    json_t *stages = json_array();
    char input_path[256], output_path[256];
    char gen_script[50];

    // Cria o nome do script de geração dinamicamente
    snprintf(gen_script, sizeof(gen_script), "gen_%s.py", particle_name);

    const char *stage_names[] = {
        "step_1", "step_2", "step_3", "step_4", "step_5"
    };
    const char *scripts[] = {
        gen_script, "simu_trf.py", "digit_trf.py", "reco_trf.py", "ntuple_trf.py"  // Usa o nome dinâmico aqui
    };

    for (int i = 0; i < 5; ++i) {
        printf("Adding stage: %s\n", stage_names[i]);

        json_t *step = json_object();
        json_object_set_new(step, "name", json_string(stage_names[i]));
        json_object_set_new(step, "script", json_string(scripts[i]));

        json_t *args = json_object();
        if (i == 0) {
            json_object_set_new(args, "seed", json_string("%SEED"));
            json_object_set_new(args, "run-number", json_string("%RUN_NUMBER"));
            json_object_set_new(args, "event-numbers", json_string("%EVENT_NUMBERS"));
        } else if (i == 1) {
            json_object_set_new(args, "enable-magnetic-field", json_string("None"));
            json_object_set_new(args, "timeout", json_integer(30));
            json_object_set_new(args, "number-of-threads", json_string("%CPU_CORES"));
        }

        json_object_set_new(step, "extra_args", args);

        if (i > 0) {
            snprintf(input_path, sizeof(input_path), 
                   "%%JOB_WORKAREA/step_%d/%s.%s.%%JOB_ID.root",
                   i, particle_name, 
                   (i == 1) ? "EVT" : (i == 2) ? "HIT" : (i == 3) ? "ESD" : "AOD");
            json_object_set_new(step, "input", json_string(input_path));
        }

        snprintf(output_path, sizeof(output_path), 
               "%%JOB_WORKAREA/step_%d/%s.%s.%%JOB_ID.root",
               i+1, particle_name,
               (i == 0) ? "EVT" : (i == 1) ? "HIT" : (i == 2) ? "ESD" : (i == 3) ? "AOD" : "NTUP");
        json_object_set_new(step, "output", json_string(output_path));

        json_array_append_new(stages, step);
    }

    return stages;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <production_card_path> <output_dir> <chunk_size> <MAX_EVENT_NUM> <particle_name>\n", argv[0]);
        return 1;
    }

    const char *production_card_path = argv[1];
    const char *output_dir = argv[2];
    int chunk_size = atoi(argv[3]);
    int MAX_EVENT_NUM = atoi(argv[4]);
    const char *particle_name = argv[5];

    if (create_directory(output_dir) != 0 && errno != EEXIST) {
        perror("Failed to create output directory");
        return 1;
    }

    FILE *production_card_file = fopen(production_card_path, "r");
    if (!production_card_file) {
        perror("Failed to open production card");
        return 1;
    }

    json_error_t error;
    json_t *root = json_loadf(production_card_file, 0, &error);
    fclose(production_card_file);
    if (!root) {
        printf("Error parsing JSON: %s\n", error.text);
        return 1;
    }

    json_t *run_obj = json_object_get(root, "run");
    int nov = json_integer_value(json_object_get(run_obj, "nov"));
    int num_jobs = nov / chunk_size;

    for (int i = 0; i < num_jobs; i++) {
        printf("Creating job %d with events [%d - %d] for particle %s\n", 
              i, i * chunk_size, (i + 1) * chunk_size - 1, particle_name);

        json_t *job_obj = json_object();

        json_t *event_numbers = json_array();
        for (int j = 0; j < chunk_size; j++) {
            json_array_append_new(event_numbers, json_integer(i * chunk_size + j));
        }

        json_object_set_new(job_obj, "event_numbers", event_numbers);
        json_object_set_new(job_obj, "job_id", json_integer(i));
        json_object_set_new(job_obj, "seed", json_integer(512 * (i + 1)));
        json_object_set_new(job_obj, "particle_name", json_string(particle_name));

        json_t *run_obj_inner = json_object();
        json_object_set_new(run_obj_inner, "nov", json_integer(MAX_EVENT_NUM));
        json_object_set_new(run_obj_inner, "nov_per_job", json_integer(chunk_size));
        json_object_set_new(run_obj_inner, "run_number", json_integer(20250122));
        json_object_set_new(run_obj_inner, "seed", json_integer(512));
        json_object_set_new(job_obj, "run", run_obj_inner);

        // 🔥 Adiciona os stages aqui!
        json_object_set_new(job_obj, "stages", build_stages(particle_name));

        char output_file_path[1024];
        snprintf(output_file_path, sizeof(output_file_path), "%s/job.%d.json", 
                output_dir, i);
        FILE *output_file = fopen(output_file_path, "w");
        if (!output_file) {
            perror("Failed to open output file");
            json_decref(job_obj);
            json_decref(root);
            return 1;
        }

        json_dumpf(job_obj, output_file, JSON_INDENT(4));
        fclose(output_file);
        json_decref(job_obj);
    }

    json_decref(root);
    return 0;
}

