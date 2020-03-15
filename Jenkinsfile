pipeline {
    agent any

    post {
        success {
            updateGitlabCommitStatus name: 'build', state: 'success'
            updateGitlabCommitStatus name: 'test', state: 'success'
            updateGitlabCommitStatus name: 'deploy', state: 'success'
        }
        failure {
            updateGitlabCommitStatus name: 'build', state: 'failed'
            updateGitlabCommitStatus name: 'test', state: 'failed'
            updateGitlabCommitStatus name: 'deploy', state: 'failed'
        }
    }

    options {
        gitLabConnection('GitLab')
        gitlabBuilds(builds: ['build', 'test', 'deploy'])
    }

    triggers {
        gitlab(
            triggerOnPush: true,
            triggerOnMergeRequest: true,
            branchFilterType: 'All',
            addNoteOnMergeRequest: true,
            addCiMessage: true
        )
    }

    stages {
        stage('build'){
            steps {
                echo "BUILD"
            }
        }

        stage('test'){
            steps {
                echo "TEST"
            }
        }

        stage('deploy'){
            steps {
                echo "DEPLOY"
            }
        }
    }
}