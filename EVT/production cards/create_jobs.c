#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
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

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <production_card.json> <output_dir> <chunk_size> <max_events> <particle_name>\n", argv[0]);
        return 1;
    }

    const char *production_card_path = argv[1];
    const char *output_dir = argv[2];
    int chunk_size = atoi(argv[3]);
    int max_events = atoi(argv[4]);  // Now actually used
    const char *particle_name = argv[5];

    // Validate input
    if (chunk_size <= 0 || max_events <= 0) {
        fprintf(stderr, "Error: chunk_size and max_events must be positive integers\n");
        return 1;
    }

    // Create output directory
    if (create_directory(output_dir) != 0 && errno != EEXIST) {
        perror("Error creating output directory");
        return 1;
    }

    // Load production card
    json_error_t error;
    json_t *root = json_load_file(production_card_path, 0, &error);
    if (!root) {
        fprintf(stderr, "JSON error on line %d: %s\n", error.line, error.text);
        return 1;
    }

    // Get run and stages info
    json_t *run_obj = json_object_get(root, "run");
    json_t *stages = json_object_get(root, "stages");
    if (!json_is_object(run_obj) || !json_is_array(stages)) {
        fprintf(stderr, "Malformed production card: missing run object or stages array\n");
        json_decref(root);
        return 1;
    }

    // Calculate number of jobs needed
    int nov = json_integer_value(json_object_get(run_obj, "nov"));
    int num_jobs = (nov + chunk_size - 1) / chunk_size;  // Round up division
    
    // Ensure we don't exceed max_events
    if (nov > max_events) {
        num_jobs = (max_events + chunk_size - 1) / chunk_size;
        fprintf(stderr, "Warning: Limiting to %d events (requested %d)\n", max_events, nov);
    }

    // Generate job files
    for (int i = 0; i < num_jobs; ++i) {
        json_t *job = json_object();
        if (!job) {
            fprintf(stderr, "Error creating job object\n");
            json_decref(root);
            return 1;
        }

        // Calculate actual chunk size for last job
        int current_chunk = chunk_size;
        if (i == num_jobs - 1) {
            int remaining = (nov > max_events ? max_events : nov) - i * chunk_size;
            current_chunk = remaining > 0 ? remaining : chunk_size;
        }

        // Set job metadata
        json_object_set_new(job, "job_id", json_integer(i));
        json_object_set_new(job, "seed", json_integer(512 * (i + 1)));
        json_object_set_new(job, "particle_name", json_string(particle_name));

        // Create event numbers array
        json_t *event_numbers = json_array();
        for (int j = 0; j < current_chunk; ++j) {
            json_array_append_new(event_numbers, json_integer(i * chunk_size + j));
        }
        json_object_set_new(job, "event_numbers", event_numbers);

        // Copy run configuration
        json_t *run_copy = json_deep_copy(run_obj);
        json_object_set_new(run_copy, "nov_per_job", json_integer(current_chunk));
        json_object_set_new(job, "run", run_copy);

        // Copy stages
        json_object_set_new(job, "stages", json_deep_copy(stages));

        // Save job file
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/job.%d.json", output_dir, i);
        
        if (json_dump_file(job, filepath, JSON_INDENT(4)) != 0) {
            fprintf(stderr, "Error writing job file %s\n", filepath);
            json_decref(job);
            json_decref(root);
            return 1;
        }
        
        json_decref(job);
    }

    json_decref(root);
    printf("✔ Generated %d job files in %s\n", num_jobs, output_dir);
    return 0;
}
